#include "rpc_provider.h"
#include "rpc_application.h"
#include "rpc_header.pb.h"
#include "rpc_logger.h"
#include <iostream>

void RPCprovider::NotifyService(google::protobuf::Service *service)
{
    // 
    ServiceInfo service_info;
    // 
    const google::protobuf::ServiceDescriptor *psd = service->GetDescriptor();
    // 
    std::string service_name = psd->name();
    // 
    int method_count = psd->method_count();
    std::cout << "service_name: " << service_name << std::endl;

    for (int i = 0; i < method_count; i++)
    {
        const google::protobuf::MethodDescriptor *pmd = psd->method(i);
        std::string method_name = pmd->name();
        std::cout << "method_name: " << method_name << std::endl;
        service_info.method_map.emplace(merhod_name, pmd);
    }
    service_info.service = service;
    service_map.emplace(service_name, service_info);
}

void RPCprovider::Run()
{
    std::string ip = RPCapplication::GetInstance().GetConfig().Load("rpcserverip");
    int port = atoi(RPCapplication::GetInstance.GetConfig.Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "RPCprovider");

    server->setConnectionCallback(std::bind(&RPCprovider::OnConnection, this, std::placeholders::_1));
    server->setMessageCallback(std::bind(&RPCprovider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::palceholders::_3));

    

}