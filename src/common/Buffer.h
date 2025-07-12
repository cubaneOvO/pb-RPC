#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>
#include "src/common/log.h"

namespace pbrpc
{
   /* class Buffer
    {
    private:
        std::string buf_;
        const uint16_t sep_; // 报文的分隔符: 0-无分隔符（应用在固定长度/视频会议）1-四字节报头 2-"\r\n\r\n"(http协议)
    public:
        Buffer(int sep = 1);
        ~Buffer() = default;
        void append(const char *, size_t);        // 追加数据
        void appendWithsep(const char *, size_t); // 追加数据并根据报文格式进行处理
        size_t size();                            // 返回buf_大小
        const char *data();                       // 返回buf_首地址
        void clear();                             // 清空buf_
        void erase(size_t pos, size_t n);         // 删除pos开始的n个字符
        bool pickMessage(std::string &);          // 将报文从buffer中取出
    };*/

    class Buffer
    {
    private:
        std::vector<char> buffer_;
        size_t readIndex_;
        size_t writeIndex_;

        void makeSpace(size_t len)//调整空间
        {
            if (writeableBytes() + prependableBytes() < len)
            {
                // 扩充空间
                buffer_.resize(writeIndex_+len);
            }
            else
            {
                // 移动
                size_t readable = readableBytes();
                // 把 begin() + readerIndex_ 到 begin()+writerIndex_ 拷贝到 begin()+kCheapPrepend
                std::copy(buffer_.begin()+readIndex_,
                          buffer_.begin()+writeIndex_,
                          buffer_.begin());
                readIndex_ = 0;
                writeIndex_ = readIndex_ + readable;
            }
        }

    public:

        static const size_t kInitialSize = 1024;

        
        Buffer()
        : buffer_(kInitialSize),
            readIndex_(0),
            writeIndex_(0)
        {

        }

        size_t getReadIndex()const {
            return readIndex_;
        }

        size_t getWriteIndex()const {
            return writeIndex_;
        }

        const std::vector<char>& getBuffer()const{
            return buffer_;
        }

        size_t readableBytes()const {
            return writeIndex_ - readIndex_;
        }

        size_t writeableBytes()const {
            return buffer_.size() - writeIndex_;
        }

        size_t prependableBytes() const {//空闲空间
            return readIndex_; 
        }

        void hasWritten(size_t len){
            writeIndex_ += len; 
        }

        void retrieve(size_t len)
        {
            assert(len <= readableBytes());
            if (len < readableBytes())
            {
                readIndex_ += len;
            }
            else if(len == readableBytes())
            {
                readIndex_ = 0;
                writeIndex_ = 0;
            }
        }

        ssize_t readFd(int fd, int* savedErrno);//直接从fd中读取数据到buffer中
        ssize_t writeFd(int fd, int* savedErrno);//直接从fd中读取数据到buffer中

        void writeToBuffer(const char* data, size_t len){
            if(writeableBytes() < len){
                makeSpace(len);
            }
            std::copy(data, data+len, buffer_.begin()+writeIndex_);
            hasWritten(len);
        }

        void readFromBuffer(std::vector<char>&re, size_t len){
            assert(len <= readableBytes());
            std::copy(buffer_.begin()+readIndex_, buffer_.begin()+readIndex_+len, re.begin());
            retrieve(len);
            return;
        }

        std::string retrieveAsString(size_t len)
        {
            assert(len <= readableBytes());
            std::string result(&buffer_[readIndex_], len);
            retrieve(len);
            return result;
        }

        int32_t readInt32()
        {
            int32_t result = peekInt32();
            retrieve(sizeof(int32_t));
            return result;
        }

        int16_t readInt16()
        {
            int16_t result = peekInt16();
            retrieve(sizeof(int16_t));
            return result;
        }

        int8_t readInt8()
        {
            int8_t result = peekInt8();
            retrieve(sizeof(int8_t));
            return result;
        }

        int32_t peekInt32() const
        {
            assert(readableBytes() >= sizeof(int32_t));
            int32_t be32 = 0;
            ::memcpy(&be32, &buffer_[readIndex_], sizeof be32);
            return be32toh(be32);
        }

        int16_t peekInt16() const
        {
            assert(readableBytes() >= sizeof(int16_t));
            int16_t be16 = 0;
            ::memcpy(&be16, &buffer_[readIndex_], sizeof be16);
            return be16toh(be16);
        }

        int8_t peekInt8() const
        {
            assert(readableBytes() >= sizeof(int8_t));
            int8_t x = buffer_[readIndex_];
            return x;
        }

        void appendInt32(int32_t x)
        {
            int32_t be32 = htobe32(x);
            writeToBuffer(reinterpret_cast<const char*>(&be32), sizeof be32);
        }

        void appendInt16(int16_t x)
        {
            int16_t be16 = htobe32(x);
            writeToBuffer(reinterpret_cast<const char*>(&be16), sizeof be16);
        }

        void appendInt8(int8_t x)
        {
            writeToBuffer(reinterpret_cast<const char*>(&x), sizeof x);
        }
    };
}
