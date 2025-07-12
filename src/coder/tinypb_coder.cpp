#include "src/coder/tinypb_coder.h"
namespace pbrpc
{
    // 将messge中的对象转化为字节流，写入到buffer
    void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr> &messages, Buffer &out_buffer)
    {
        for(auto it : messages){
            std::shared_ptr<TinyPBProtocol> pkg = std::dynamic_pointer_cast<TinyPBProtocol>(it);

            if(pkg->msg_id_.empty()){//TODO: 零时编一个
                pkg->msg_id_ = "123456789";
            }
            //XLOG_DEBUG("msg_id = {}", pkg->msg_id_);
            //int pk_len = 2 + sizeof(int32_t) * 6 + pkg->msg_id_.length() + pkg->method_name_.length() + pkg->error_info_.length() + pkg->pb_data_.length();
            int pk_len = 3 + sizeof(int32_t) * 5 + pkg->msg_id_.length() + pkg->method_name_.length() + pkg->pb_data_.length();
            //XLOG_DEBUG("pk_len = {}", pk_len);

            out_buffer.appendInt8(TinyPBProtocol::PB_START);
            out_buffer.appendInt32(pk_len);
            out_buffer.appendInt8(pkg->msg_type_);
            out_buffer.appendInt32(pkg->msg_id_.length());
            out_buffer.writeToBuffer(pkg->msg_id_.c_str(), pkg->msg_id_.length());
            out_buffer.appendInt32(pkg->method_name_.length());
            out_buffer.writeToBuffer(pkg->method_name_.c_str(), pkg->method_name_.length());
            out_buffer.appendInt32(pkg->error_code_);
            //out_buffer.appendInt32(pkg->error_info_.length());
            //out_buffer.writeToBuffer(pkg->error_info_.c_str(), pkg->error_info_.length());
            out_buffer.writeToBuffer(pkg->pb_data_.c_str(), pkg->pb_data_.length());
            //TODO: 处理校验和
            out_buffer.appendInt32(pkg->check_sum_);
            out_buffer.appendInt8(TinyPBProtocol::PB_END);

            XLOG_DEBUG("encode message {} success", pkg->msg_id_);
        }
        
    }

    // 遍历buffer找到PB_START.找到之后根据协议进行解析
    void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr> &out_messages, Buffer &buffer)
    {
        while (true)
        {
            int32_t pk_len = 0;
            bool parse_success = false;
            const std::vector<char> &buf = buffer.getBuffer();

            int start_index = buffer.getReadIndex();
            int end_index = 0;
            for (int n = buffer.getWriteIndex(); start_index < n; start_index++)
            {
                // 直接掠过非begin字节
                if (buf[start_index] == TinyPBProtocol::PB_START)
                {
                    if (start_index + sizeof(int32_t) < n)
                    { // 可以取出四字节
                        ::memcpy(&pk_len, &buf[start_index + 1], sizeof pk_len);
                        pk_len = be32toh(pk_len);
                        //XLOG_DEBUG("get pk_len = {}", pk_len);
                        end_index = start_index + pk_len - 1;
                        if (end_index >= n)
                        { // 包不完整
                            break;
                        }
                        else if (buf[end_index] == TinyPBProtocol::PB_END)
                        { // 可以读完
                            parse_success = true;
                            break;
                        }
                    }
                    else
                        break; // 连四字节都取不出包肯定不完整
                }
            }
            buffer.retrieve(start_index - buffer.getReadIndex()); // 调整起始字节
            
            if (parse_success)
            {
                //int parse_success = true;
                std::shared_ptr<TinyPBProtocol> pkg = std::make_shared<TinyPBProtocol>();
                buffer.readInt8(); // 取出PB_START
                pkg->pk_len_ = buffer.readInt32();

                int msg_type_index = start_index + sizeof(char) + sizeof(pkg->pk_len_) + sizeof(pkg->msg_type_);
                if(msg_type_index >= end_index)
                {
                    XLOG_ERROR("parse error, msg_type_index:{} >= end_index:{}", msg_type_index, end_index);
                    parse_success = false;
                }
                else 
                    pkg->msg_type_ = buffer.readInt8();

                //int msg_id_len_index = start_index + sizeof(char) + sizeof(pkg->pk_len_) + sizeof(pkg->msg_id_len_);
                int msg_id_len_index = msg_type_index + sizeof(pkg->msg_id_len_);
                if (msg_id_len_index >= end_index)
                {
                    XLOG_ERROR("parse error, msg_id_len_index:{} >= end_index:{}", msg_id_len_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->msg_id_len_ = buffer.readInt32();
                    

                int msg_id_index = msg_id_len_index + pkg->msg_id_len_;
                if (msg_id_index >= end_index)
                {
                    XLOG_ERROR("parse error, msg_id_index:{} >= end_index:{}", msg_id_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->msg_id_ = buffer.retrieveAsString(pkg->msg_id_len_);

                int method_name_len_index = msg_id_index + sizeof(pkg->method_name_len_);
                if (method_name_len_index >= end_index)
                {
                    XLOG_ERROR("parse error, method_name_len_index:{} >= end_index:{}", method_name_len_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->method_name_len_ = buffer.readInt32();

                int method_name_index = method_name_len_index + pkg->method_name_len_;
                if (method_name_index >= end_index)
                {
                    XLOG_ERROR("parse error, method_name_index:{} >= end_index:{}", method_name_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->method_name_ = buffer.retrieveAsString(pkg->method_name_len_);

                int error_code_index = method_name_index + sizeof(pkg->error_code_);
                if (error_code_index >= end_index)
                {
                    XLOG_ERROR("parse error, error_code_index:{} >= end_index:{}", error_code_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->error_code_ = buffer.readInt32();

                /*int error_info_len_index = error_code_index + sizeof(pkg->error_info_len_);
                if (error_info_len_index >= end_index)
                {
                    XLOG_ERROR("parse error, error_info_len_index:{} >= end_index:{}", error_info_len_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->error_info_len_ = buffer.readInt32();

                int error_info_index = error_info_len_index + pkg->error_info_len_;
                if (error_info_index >= end_index)
                {
                    XLOG_ERROR("parse error, error_info_index:{} >= end_index:{}", error_info_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->error_info_ = buffer.retrieveAsString(pkg->error_info_len_);
                */

                //int32_t pb_data_len = pkg->pk_len_ - pkg->msg_id_len_ - pkg->method_name_len_ - pkg->error_info_len_ - sizeof(int32_t) * 6 - 2;

                //int pb_data_index = error_info_index + pb_data_len;

                
                int32_t pb_data_len = pkg->pk_len_ - pkg->msg_id_len_ - pkg->method_name_len_ -  sizeof(int32_t) * 5 - 3;

                int pb_data_index = error_code_index + pb_data_len;
                if (pb_data_index >= end_index)
                {
                    XLOG_ERROR("parse error, pb_data_index:{} >= end_index:{}", pb_data_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->pb_data_ = buffer.retrieveAsString(pb_data_len);

                int check_sum_index = pb_data_index + sizeof(pkg->check_sum_);
                if (check_sum_index > end_index)
                {
                    XLOG_ERROR("parse error, check_sum_index:{} >= end_index:{}", check_sum_index, end_index);
                    parse_success = false;
                }
                else
                    pkg->check_sum_ = buffer.readInt32();

                //TODO: 处理校验和

                if(parse_success){//解析成功
                    buffer.readInt8(); // 取出PB_END
                    pkg->parse_success = true;
                }
                else{
                    XLOG_ERROR("parse failed");
                    buffer.retrieve(end_index - buffer.getReadIndex() + 1); // 调整起始字节
                }
                out_messages.push_back(pkg);
            }
            else break;
        }
    }
}
