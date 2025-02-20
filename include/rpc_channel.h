#ifndef _rpc_channel_H
#define _rpc_channel_H

#include <google/protobuf/service.h>
#include "zookeeper_util.h"
//RPCchannel类是继承于google::protobuf::RpcChannel
//目的是为了给客户端进行方法调用的时候来统一接收的
class RPCchannel : public google::protobuf::RpcChannel
{
public:
    RPCchannel(bool connectNow);
    virtual ~RPCchannel(){}
    // override可以验证是否为虚函数
    void CallMethod(const ::google::protobuf::MethodDescriptor *method,
                    ::google::protobuf : RpcController *controller,
                    const ::google::protobuf::Message *request,
                    ::google::protobuf::Message *response,
                    ::google::protobuf::Closure *done) override;

private:
    int m_clientfd; // 存放客户端套接字
    std::string service_name;
    std::string m_ip;
    uint16_t m_port;
    std::string method_mane;
    int m_idx; // 用来划分服务器IP和port的下标
    bool NewConnect(const char *ip, uint16_t port);
    std::string QueryServiceHost(ZKclient *zkclient,
                                 std::string service_name,
                                 std::string method_name,
                                 int &idx);
};

#endif