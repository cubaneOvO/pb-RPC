#pragma once
#include "etcd/Client.hpp"
#include "etcd/KeepAlive.hpp"
#include "etcd/Response.hpp"
#include "etcd/Watcher.hpp"
#include "etcd/Value.hpp"

#include "src/common/log.h"
#include "src/common/Config.h"

namespace pbrpc{
    class RpcDiscovery
    {
    public:
        using ptr = std::shared_ptr<RpcDiscovery>;
        using NotyfiyCallBack = std::function<void(std::string, std::string)>;
        RpcDiscovery(const NotyfiyCallBack& put_cb,const NotyfiyCallBack& del_cb) : 
            _client(std::make_shared<etcd::Client>(CConfig::GetInstance()->etcd_url_)),
            _put_cb(put_cb),
            _del_cb(del_cb)
        {
            auto response = _client->ls(CConfig::GetInstance()->etcd_base_dir_).get();
            if (response.is_ok() == false)
            {
                XLOG_ERROR("get data error: {}",response.error_message());
            }

            size_t sz = response.keys().size();
            for (int i = 0; i < sz; ++i)
            {
                if (_put_cb)
                    _put_cb(response.key(i), response.value(i).as_string());
            }
            _watcher = std::make_shared<etcd::Watcher>(*_client.get(), CConfig::GetInstance()->etcd_base_dir_,
                std::bind(&RpcDiscovery::callBack, this, std::placeholders::_1), true);
        }
        ~RpcDiscovery()
        {
            _watcher->Cancel();
        }

    private:
        void callBack(const etcd::Response& response)
        {
            if (response.is_ok() == false)
            {
                XLOG_ERROR("get error notify: {}",response.error_message());
                return;
            }
            for (auto& event : response.events())
            {
                if (event.event_type() == etcd::Event::EventType::PUT)
                {
                    XLOG_INFO("Service online {}:{}",event.kv().key(), event.kv().as_string());
                    if (_put_cb)
                        _put_cb(event.kv().key(), event.kv().as_string());
                }
                else if (event.event_type() == etcd::Event::EventType::DELETE_)
                {
                    XLOG_INFO("Service down {}:{}",event.kv().key(), event.kv().as_string());
                    if (_del_cb)
                        _del_cb(event.kv().key(), event.kv().as_string());
                }
            }
        }
        NotyfiyCallBack _put_cb;
        NotyfiyCallBack _del_cb;
        std::shared_ptr<etcd::Watcher> _watcher;
        std::shared_ptr<etcd::Client> _client;
    };
}