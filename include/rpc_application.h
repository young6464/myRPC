#ifndef _rpc_application_H
#define _rpc_application_H

#include <mutex>
#include "rpc_config.h"
#include "rpc_channel.h"
#include "rpc_controller.h"

// rpc基础类，负责框架的一些初始化操作
class RPCapplication
{
public:
    // 初始化
    static void Init(int argc, char **argv);
    // 用于获取单例模式的实列
    static RPCapplication &GetInstance();
    // 删除单例模式的实例
    static void DeleteInstance();
    // 获取配置
    static RPCconfig &GetConfig();

private:
    static RPCconfig m_config;
    // 全局唯一单例访问对象
    static RPCapplication *m_application;
    // 互斥锁
    static std::mutex m_mutex;
    // 构造函数
    RPCapplication() {}
    ~RPCapplication() {}
    // 禁止拷贝构造函数(c++11语法)
    RPCapplication(const RPCapplication &) = delete;
    RPCapplication(RPCapplication &&) = delete;
};
#endif