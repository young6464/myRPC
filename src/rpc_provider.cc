#include "rpc_provider.h"
#include "rpc_application.h"
#include "rpc_header.pb.h"
#include "rpc_logger.h"
#include <iostream>

/// @brief 此函数用于将服务对象注册到RPC provider中
/// @param service的参数类型是google::protobuf::Service，这样设置是因为所有的服务类都继承自google::protobuf::Service类，这样便可通过基类指针指向子类对象，实现动态多态
void RPCprovider::NotifyService(google::protobuf::Service *service)
{
    // 服务端需要知道对方想要调用的具体服务对象和方法，这类信息会保存在ServiceInfo结构体中
    ServiceInfo service_info;
    // 通过动态多态调用service->GetDescriptor()，获取由Protobuf生成的服务对象的描述信息
    // google::protobuf::ServiceDescriptor是一个描述RPC服务的类，通过此类可以获取该服务中定义的方法列表，进而处理相应的注册和管理操作
    const google::protobuf::ServiceDescriptor *psd = service->GetDescriptor();
    // 获取服务名称，服务名是唯一的标识符，用于区分不同的服务
    std::string service_name = psd->name();
    // 获取服务端对象service的方法数量
    int method_count = psd->method_count();
    std::cout << "service_name: " << service_name << std::endl;

    for (int i = 0; i < method_count; i++)
    {
        // 获取服务端对象service的每个方法的描述信息
        const google::protobuf::MethodDescriptor *pmd = psd->method(i);
        std::string method_name = pmd->name();
        std::cout << "method_name: " << method_name << std::endl;
        service_info.method_map.emplace(method_name, pmd);
    }
    service_info.service = service;
    service_map.emplace(service_name, service_info);
}

/// @brief 启动RPC服务节点，开始提供RPC远程网络调用服务
void RPCprovider::Run()
{
    // 读取配置文件RPCServer中的IP和端口信息，并通过muduo库创建一个address对象
    std::string ip = RPCapplication::GetInstance().GetConfig().Load("rpcserverip");
    int port = atoi(RPCapplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建一个TcpServer对象，用于处理网络事件
    std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "RPCprovider");

    // 绑定连接回调函数和消息回调函数。此处分离了网络连接业务和消息处理业务
    server->setConnectionCallback(std::bind(&RPCprovider::OnConnection, this, std::placeholders::_1));
    server->setMessageCallback(std::bind(&RPCprovider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置muduo库的线程数，可根据实际情况调整
    server->setThreadNum(4);

    // 把当前RPC节点上要发布的服务全部注册到Zookeeper中，让RPC Client客户端可以通过Zookeeper来发现服务
    ZKclient zk_client;
    zk_client.Start();

    // service_map是永久节点，method_map是临时节点
    for (auto &sp : service_map)
    {
        // service_map在zk中的目录为“/”+service_name
        std::string service_path = "/" + sp.first;
        zk_client.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.method_map)
        {
            // method_map在zk中的目录为“/”+service_name+“/”+method_name
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            // 打印服务的IP和端口信息
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);

            // ZOO_EPHEMERAL表示临时节点，临时节点在客户端断开连接后会自动删除
            // 创建临时节点，存储服务的IP和端口信息
            zk_client.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }
    // 打印RPC服务启动信息
    std::cout << "RPCprovider start service at ip:" << ip << "port:" << port << std::endl;
    // 启动muduo库的事件循环，监听连接请求
    server->start();
    event_loop.loop();
}

/// @brief 处理连接事件的回调函数
void RPCprovider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 如果连接关闭则直接将其断开
        conn->shutdown();
    }
}

/**
 * 在框架内，RPCprovider和RPCconsumer之间协商好通信使用的协议为Protobuf数据类型
 * OnMessage函数会解析接收到的Protobuf数据包，提取出RPC请求的服务名、方法名和参数等信息,
 * 然后根据服务名和方法名查找对应的服务对象和方法描述符，创建请求和响应消息对象,
 * 最后调用服务对象的CallMethod方法，执行远程过程调用，并通过回调函数SendRPCResponse将响应结果发送回去
 */
void RPCprovider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp receive_time)
{
    std::cout << "OnMessage" << std::endl;
    // 网络上接受远程RPC调用请求的字符流
    // 一般情况下，RPC请求数据包的格式为：
    // [header_size][header_str][args_str]
    // header_size是RPC请求数据包头的大小，header_str是RPC请求数据包头的内容（一般为服务对象及其方法），args_str是RPC请求数据包的参数内容
    std::string recv_buf = buffer->retrieveAllAsString();

    // 使用Protobuf的CodedInputStream反序列化接收到的RPC请求数据包
    google::protobuf::io::ArrayInputStream raw_input(recv_buf.data(), recv_buf.size());
    google::protobuf::io::CodedInputStream coded_input(&raw_input);

    // 解析header_size
    uint32_t header_size{};
    coded_input.ReadVarint32(&header_size);
    // 根据header_size读取数据包头的原始字符流，反序列化数据，得到RPC请求的服务名、方法名和参数大小等详细信息
    std::string rpc_header_str;
    myRPC::RPCHeader rpc_header;
    std::string service_name;
    std::string method_name;
    uint32_t args_size{};
    // PushLimit用于限制读取的字节数，防止读取过多数据导致内存溢出
    google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
    coded_input.ReadString(&rpc_header_str, header_size);
    // 读取完header_size后，PopLimit用于恢复之前的限制，以便安全地读取其他数据
    coded_input.PopLimit(msg_limit);
    if (rpc_header.ParseFromString(rpc_header_str))
    {
        service_name = rpc_header.service_name();
        method_name = rpc_header.method_name();
        args_size = rpc_header.args_size();
    }
    else
    {
        RPClogger::Error("rpc_header parse error");
        return;
    }
    // 直接读取args_size大小的字符串数据
    std::string args_str;
    bool read_args_success = coded_input.ReadString(&args_str, args_size);
    if (!read_args_success)
    {
        RPClogger::Error("read args error");
        return;
    }

    // 获取serice对象和method对象
    auto it = service_map.find(service_name);
    if (it == service_map.end())
    {
        std::cout << service_name << "is not exist!" << std::endl;
        return;
    }
    auto mit = it->second.method_map.find(method_name);
    if (mit == it->second.method_map.end())
    {
        std::cout << service_name << "." << method_name << "is not exist!" << std::endl;
        return;
    }
    google::protobuf::Service *service = it->second.service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成RPC方法调用请求的request和response对象
    // 通过GetRequestPrototype方法，可以根据MethodDescriptor方法描述符动态获取对应的请求消息类型，并实例化该类型的对象
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << service_name << "." << method_name << "parse error!" << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetRequestPrototype(method).New();

    // 使用google::protobuf::Closure回调函数来处理RPC响应
    google::protobuf::Closure *done = google::protobuf::NewCallback<RPCprovider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this, &RPCprovider::SendRPCResponse,
                                                                                                 conn, response);

    service->CallMethod(method, nullptr, request, response, done);
}

/// @brief 将RPC响应结果发送回RPC调用方
void RPCprovider::SendRPCResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializePartialToString(&response_str))
    {
        // 序列化成功后，通过muduo库的TcpConnection发送RPC方法调用的响应结果给远程调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize error!" << std::endl;
    }
    // conn->shutdown();
}

RPCprovider::~RPCprovider()
{
    std::cout << "~RPCprovider()" << std::endl;
    event_loop.quit();
}