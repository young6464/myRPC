#ifndef _rpc_logger_H
#define _rpc_logger_H

#include <glog/logging.h>
#include <string>
// 采用RAII(资源获取即初始化)的思想
class RPClogger
{
public:
    // 构造函数，自动初始化gLOG
    explicit RPClogger(const char *argv0)
    {
        google::InitGoogleLogging(argv0);
        FLAGS_colorlogtostderr = true; // 启用彩色日志，输出到标准错误
        FLAGS_logtostderr = true;      // 日志默认输出到标准错误
    }
    // 析构函数，自动关闭gLOg
    ~RPClogger()
    {
        google::ShutdownGoogleLogging();
    }
    // 提供静态日志方法，记录不同级别的信息
    // 记录一般级别信息
    static void Info(const std::string &message)
    {
        LOG(INFO) << message;
    }
    // 记录警告级别信息
    static void Warning(const std::string &message)
    {
        LOG(WARNING) << message;
    }
    // 记录错误级别信息
    static void Error(const std::string &message)
    {
        LOG(ERROR) << message;
    }
    // 记录致命错误信息
    static void Fatal(const std::string &message)
    {
        LOG(FATAL) << message;
    }

private:
    // 删除拷贝构造函数和赋值运算符，防止复制RPClogger对象
    RPClogger(const RPClogger &) = delete;
    RPClogger &operator=(const RPClogger &) = delete;
};

#endif