#include "src/common/Buffer.h"

namespace pbrpc {
    /*Buffer::Buffer(int sep):sep_(sep){

    }
    
    void Buffer::append(const char *data, size_t size){
    //std::cout <<  data << std::endl;
        buf_.append(data, size);
               //std::cout <<  buf_ << std::endl;
    }
    
    void Buffer::appendWithsep(const char *data, size_t size){
        if(sep_ == 1){
            buf_.append((const char*)&size, 4);
            buf_.append(data, size);
     
        }
        else if(sep_ == 0){
            buf_.append(data, size);
        }
        //可扩展http协议格式报文
    }
    
    size_t Buffer::size()
    {
        return buf_.size();
    }
    
    const char *Buffer::data()
    {
        return buf_.data();
    }
    
    void Buffer::clear(){
        buf_.clear();
    }
    
    void Buffer::erase(size_t pos, size_t n){
        buf_.erase(pos, n);
    }
    
    bool Buffer::pickMessage(std::string & str){
        if(buf_.size() == 0)return false;
        if(sep_ == 0){
            str = buf_;
            buf_.clear();
        }
        else if(sep_ == 1){
            int len;
            memcpy(&len, buf_.data(), 4);//取报文长度
            if(buf_.size() < len+4)//数据接收不完整
                return false;
            str = buf_.substr(4,len);//将报文取出
            buf_.erase(0, len+4);
        }
        return true;
    }*/

    template<typename To, typename From>
    inline To implicit_cast(From const &f) {
        return f;
    }

    ssize_t Buffer::readFd(int fd, int* savedErrno){
        // saved an ioctl()/FIONREAD call to tell how much to read
        // 节省一次ioctl系统调用（获取有多少可读数据）
        char extrabuf[65536];
        struct iovec vec[2];
        // 可写的大小
        const size_t writable = writeableBytes();
    
        // 第一块缓冲区
        vec[0].iov_base = &buffer_[writeIndex_];
        vec[0].iov_len = writable;
        // 第二块缓冲区
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof extrabuf;
    
        const ssize_t n = ::readv(fd, vec, 2);   // 从 fd 里接收数据，放到 vec 中【有两块缓冲区】

        if (n < 0)
        {
            *savedErrno = errno;
        }
        else if (implicit_cast<size_t>(n) <= writable)	// 第一块缓冲区足够容纳
        {
            writeIndex_ += n;
        }
        else		// 当前缓冲区，不够容纳，因而数据被接收到了第二块缓冲区extrabuf，将其append至buffer
        {
            writeIndex_ = buffer_.size();              // 更改写指针的位置为缓冲区的末尾
            writeToBuffer(extrabuf, n-writable);    // 将缓冲区末尾添加上 extrabuf 里的内容          
        }
        return n;
    }

    ssize_t Buffer::writeFd(int fd, int* savedErrno){
        const ssize_t n = ::write(fd, &buffer_[readIndex_], readableBytes());
        if(n < 0){
            *savedErrno = errno;
        }
        else{
            retrieve(n);
        }
        return n;
    }
}


