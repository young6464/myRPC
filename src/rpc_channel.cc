#include "rpc_channel.h"
#include "rpc_application.h"
#include "rpc_controller.h"
#include "rpc_header.pb.h"
#include "rpc_logger.h"
#include "zookeeper_util.h"
#include <memory>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

std::mutex g_data_mutex;
// header_size+hservice_name method_name args_size+args_str
// 此处CallMethod是给客户端stub代理进行统一RPC方法用的数据格式序列化和网络发送，和服务
// 端的CallMethod要作区分，服务端的CallMethod只是通过动态多态调用到客户端请求的方法。

void RPCchannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller,
                            const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response,
                            ::google::protobuf::Closure *done)
{
    if (-1 == m_clientfd)
    {
        // 获取服务对象和方法名
        const google::protobuf::ServiceDescriptor *sd = method->service();
        service_name = sd->name();
        method_name = method->name();
        // 客户端(即RPC调用方)想要调用服务器上服务对象提供的方法，需要查询zookeeper上该服务所在的host信息
        ZKclient zk_client;
        zk_client.Start();
        std::string host_data = QueryServiceHost(&zk_client, service_name, method_name, m_idx);
        m_ip = host_data.substr(0, m_idx);
        std::cout << "ip: " << m_ip << std::endl;
        m_port = atoi(host_data.substr(m_idx + 1, host_data.size() - m_idx).c_str());
        std::cout << "port: " << m_port << std::endl;
        auto rt = NewConnect(m_ip.c_str(), m_port);
        if (!rt)
        {
            LOG(ERROR) << "connect server error!";
            return;
        }
        else
        {
            LOG(INFO) << "connect server success!";
        }
    }
    // 获取参数的序列化字符串长度args_size
    uint32_t args_size{};
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request fail!");
        return;
    }
    // 定义RPC报文的header
    myRPC::RPCHeader rpc_header;
    rpc_header.set_service_name(service_name);
    rpc_header.set_method_name(method_name.c_str());
    rpc_header.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpc_header.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("serialize rpc header error!");
        return;
    }
    std::string send_rpc_str;
    {
        google::protobuf::io::StringOutputStream string_output(&send_rpc_str);
        google::protobuf::io::CodedOutputStream coded_output(&string_output);
        coded_output.WriteVarint32(static_cast<uint32_t>(header_size));
        coded_output.WriteString(rpc_header_str);
    }
    send_rpc_str += args_str;

    // 发送RPC的请求
    if (-1 == send(m_clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(m_clientfd);
        char err_txt[512] = {};
        std::cout << "send error: " << strerror_r(errno, err_txt, sizeof(err_txt)) << std::endl;
        controller->SetFailed(err_txt);
        return;
    }

    // 接受RPC请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(m_clientfd, recv_buf, 1024, 0)))
    {
        char err_txt[512] = {};
        std::cout << "recv error: " << strerror_r(errno, err_txt, sizeof(err_txt)) << std::endl;
        controller->SetFailed(err_txt);
        return;
    }
    // 反序列化RPC调用响应数据
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(m_clientfd);
        char err_txt[512] = {};
        std::cout << "parse error: " << strerror_r(errno, err_txt, sizeof(err_txt)) << std::endl;
        controller->SetFailed(err_txt);
        return;
    }
    close(m_clientfd);
}

bool RPCchannel::NewConnect(const char *ip, uint16_t port)
{
    // 使用socket网络编程
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char err_txt[512] = {};
        std::cout << "socket error: " << strerror_r(errno, err_txt, sizeof(err_txt)) << std::endl;
        LOG(ERROR) << "socket error: " << err_txt;
        return false;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        char err_txt[512] = {};
        std::cout << "connect error: " << strerror_r(errno, err_txt, sizeof(err_txt)) << std::endl;
        LOG(ERROR) << "connect server error: " << err_txt;
        return false;
    }
    m_clientfd = clientfd;
    return true;
}

std::string RPCchannel::QueryServiceHost(ZKclient *zkclient,
                                         std::string service_name,
                                         std::string method_name,
                                         int &idx)
{
    std::string method_path = "/" + service_name + "/" + method_name;
    std::cout << "method_path: " << method_path << std::endl;
    std::unique_lock<std::mutex> lock(g_data_mutex);
    std::string host_data_1 = zkclient->GetData(method_path.c_str());
    lock.unlock();

    if (host_data_1 == "")
    {
        LOG(ERROR) << method_path << " is not exist!";
        return " ";
    }
    // 127.0.0.1:8000 获取到ip和port的分隔符
    idx = host_data_1.find(":");
    if (idx == -1)
    {
        LOG(ERROR) << method_path + " address is invalid!";
        return " ";
    }
    return host_data_1;
}

RPCchannel::RPCchannel(bool connect_now) : m_clientfd(-1), m_idx(0)
{
    if (!connect_now)
    {
        return;
    }
    // 判断是否连接成功
    auto rt = NewConnect(m_ip.c_str(), m_port);
    // 重连次数
    int count = 3;
    while (!rt && count--)
    {
        rt = NewConnect(m_ip.c_str(), m_port);
    }
}