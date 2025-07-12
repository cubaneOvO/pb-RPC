#pragma once
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <iostream>
#include <mutex>
#include <algorithm>
namespace pbrpc{
    struct ServerNode {
        double cpu_usage;
        std::string ip;
        int32_t port;
        
        // 用于生成唯一键的辅助函数
        std::string key() const {
            return ip + ":" + std::to_string(port);
        }
        
        // 用于最小堆的比较函数（CPU使用率高的优先级低）
        bool operator>(const ServerNode& other) const {
            return cpu_usage > other.cpu_usage;
        }
    };
    
    class ServerNodeManager {
    private:
        // 最小堆：使用shared_ptr管理生命周期
        using NodePtr = std::shared_ptr<ServerNode>;
        struct NodeComparator {
            bool operator()(const NodePtr& a, const NodePtr& b) const {
                return *a > *b; // 使用ServerNode的>运算符
            }
        };
        std::priority_queue<NodePtr, std::vector<NodePtr>, NodeComparator> minHeap;
        
        // 用于快速查找节点（IP+Port -> 节点在堆中的所有副本）
        std::unordered_map<std::string, NodePtr> nodeMap;
        
        // 用于线程安全的互斥锁
        mutable std::shared_mutex rwMutex;
        
        // 重建堆的阈值
        size_t staleCountThreshold = 100;
        size_t staleCount = 0;
    
    public:
        // 添加或更新服务器节点
        void upsertNode(const ServerNode& node) {
            std::unique_lock<std::shared_mutex> lock(rwMutex);
            
            // 生成新节点的共享指针
            NodePtr newNode = std::make_shared<ServerNode>(node);
            std::string key = newNode->key();
            
            // 取消旧节点映射
            removeNodeInternal(key);
            
            // 添加新节点到堆和映射
            minHeap.push(newNode);
            nodeMap[key] = newNode;
            
            // 检查是否需要重建堆
            if (staleCount >= staleCountThreshold && !minHeap.empty()) {
                rebuildHeap();
            }
        }
        
        // 获取CPU使用率最低的节点
        NodePtr getLowestCPUNode() {
            std::unique_lock<std::shared_mutex> lock(rwMutex);
            
            // 循环直到找到有效节点或堆为空
            while (!minHeap.empty()) {
                NodePtr top = minHeap.top();
                
                // 检查节点是否有效（未被删除）
                auto it = nodeMap.find(top->key());
                if (it != nodeMap.end() && top == it->second){
                    return top; // 找到有效节点
                }
                
                // 无效节点，弹出
                minHeap.pop();
            }
            
            return nullptr; // 堆为空
        }
        
        // 删除节点
        bool removeNode(const std::string& ip, int32_t port) {
            std::unique_lock<std::shared_mutex> lock(rwMutex);
            bool ret = removeNodeInternal(ip + ":" + std::to_string(port));
            return ret;
        }
        
        // 获取节点数量
        size_t size() const {
            std::shared_lock<std::shared_mutex> lock(rwMutex);
            return nodeMap.size();
        }
    
    private:
        // 内部删除节点实现
        bool removeNodeInternal(const std::string& key) {
            auto it = nodeMap.find(key);
            if (it == nodeMap.end()) {
                return false;
            }
            // 从映射中移除
            nodeMap.erase(it);
            staleCount ++;
            return true;
        }
        
        // 重建堆以清除无效节点
        void rebuildHeap() {
            // 清空堆并重新插入有效节点
            std::priority_queue<NodePtr, std::vector<NodePtr>, NodeComparator> emptyHeap;
            minHeap.swap(emptyHeap);
            
            for (const auto& node : nodeMap) {
                minHeap.push(node.second);
            }
            
            // 重置过时计数
            staleCount = 0;
        }
    };
}