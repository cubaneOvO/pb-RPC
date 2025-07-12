#pragma once
#include <string>
#include <random>
#include <mutex>

#include "src/common/log.h"
namespace pbrpc
{
    class MsgIdUtil
    {
    public:
        MsgIdUtil(const MsgIdUtil &) = delete;
        MsgIdUtil &operator=(const MsgIdUtil &) = delete;
        virtual ~MsgIdUtil() {};

        static MsgIdUtil *GetInstance()
        {
            static MsgIdUtil s_instance;
            return &s_instance;
        }
        std::string GetMsgId();

    private:
        MsgIdUtil() :g_msg_id_length(10){};
        const int g_msg_id_length;
        std::string t_msg_id_no;
        std::string t_max_msg_id_no;
        std::mutex mtx_;
    };
}
