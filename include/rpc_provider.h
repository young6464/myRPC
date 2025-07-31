#ifndef _rpc_provider_H
#define _rpc_provider_H

#include "zookeeper_util.h"
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <functional>
#include <string>
#include <unordered_map>

class RPCprovider
{
public:
    // 供外部使用，用来发布RPC方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    ~RPCprovider();
    // 启动RPC服务节点，开始提供RPC远程网络调用服务
    void Run();

private:
    muduo::net::EventLoop event_loop;
    struct ServiceInfo
    {
        google::protobuf::Service *service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> method_map;
    };
    // 保存服务对象和RPC方法
    std::unordered_map<std::string, ServiceInfo> service_map;
    void OnConnection(const muduo::net::TcpConnectionPtr &conn);
    void OnMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buffer, muduo::Timestamp receive_time);
    void SendRPCResponse(const muduo::net::TcpConnectionPtr &conn,
                         google::protobuf::Message *response);
};
#endif