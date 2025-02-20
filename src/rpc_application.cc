#include "rpc_application.h"
#include <cstdlib>
#include <unistd.h>

RPCconfig RPCapplication::m_config;
std::mutex RPCapplication::m_mutex;
RPCapplication *RPCapplication::m_application = nullptr;
// Init 函数用于初始化应用程序，解析命令行参数以获取配置文件路径
void RPCapplication::Init(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "format:command -i <configfile>" << std::endl;
        exit(EXIT_FAILURE);
    }
    int o;
    std::string config_file;
    // getopt函数解析命令行选项，这里只解析-i选项，用于指定配置文件
    // 如果未找到配置文件或命令行参数格式错误，将输出使用格式并退出
    while (-1 != (o = getopt(argc, argv, "i:")))
    {
        switch (o)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?': // 遇到一个不认识的选项，返回?
            std::cout << "format:command -i <configfile>" << std::endl;
            exit(EXIT_FAILURE);
            break;
        case ':': // 遇到一个缺少参数的选项（例如-i后面没有跟文件名）
            std::cout << "format:command -i <configfile>" << std::endl;
            exit(EXIT_FAILURE);
            break;
        default:
            break;
        }
    }
    // 成功解析后，加载配置文件
    m_config.LoadConfigFile(config_file.c_str());
}

// GetInstance函数用于获取RPCapplication类的单例实例
RPCapplication &RPCapplication::GetInstance()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_application == nullptr)
    {
        m_application = new RPCapplication();
        atexit(DeleteInstance); // 注册DeleteInstance函数以在程序退出时删除单例对象
    }
    return *m_application;
}

void RPCapplication::DeleteInstance()
{
    if (m_application)
    {
        delete m_application;
    }    
}

RPCconfig &RPCapplication::GetConfig()
{
    return m_config;
}