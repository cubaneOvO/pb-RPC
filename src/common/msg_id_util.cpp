#include "src/common/msg_id_util.h"

namespace pbrpc{
    std::string MsgIdUtil::GetMsgId(){
        std::unique_lock<std::mutex> lock_(mtx_);
        if(t_msg_id_no.empty() || t_msg_id_no == t_max_msg_id_no){

            thread_local std::random_device rd;
            thread_local std::mt19937_64 gen(rd());
            thread_local std::uniform_int_distribution<> dis(0, 9);

            t_max_msg_id_no = std::string(g_msg_id_length, '9');
            
            t_msg_id_no = std::string(g_msg_id_length, '0');
            for (char& c : t_msg_id_no) {
                c = dis(gen) + '0';
            }
        }
        else{
            size_t i = t_msg_id_no.length() - 1;
            while (t_msg_id_no[i] == '9' && i >= 0) {
              i--;
            }
            if (i >= 0) {
              t_msg_id_no[i] += 1;
              for (size_t j = i + 1; j < t_msg_id_no.length(); ++j) {
                t_msg_id_no[j] = '0';
              }
            }
        }
        return t_msg_id_no;
    }
}