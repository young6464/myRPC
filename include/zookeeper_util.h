#ifndef _zookeeper_util_H
#define _zookeeper_util_H

#include <string>
#include <semaphore>
#include <zookeeper/zookeeper.h>

// 封装的zk客户端
class ZKclient
{
public:
    ZKclient();
    ~ZKclient();
    // zk客户端启动zk服务端
    void Start();
    // 在zk服务端中依据参数值(路径、数据、数据长度、状态[默认为0])创建一个节点
    void Create(const char *path,
                const char *data,
                int datalen,
                int state = 0);
    // 根据指定的znode节点路径，来获取znode节点值
    std::string GetData(const char *path);

private:
    // zk客户端句柄
    zhandle_t* m_zhandle;
};

#endif