#include "zookeeper_util.h"
#include "rpc_application.h"
#include "rpc_logger.h"
#include <mutex>
#include <condition_variable>

// 全局锁
std::mutex cv_mutex;
// 信号量
std::condition_variable cv;
bool is_connected = false;

// 全局的watcher观察器，用于监听Zookeeper连接状态变化
void global_watcher(zhandle_t *zh, int type, int status, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (status == ZOO_CONNECTED_STATE)
        {
            std::lock_guard<std::mutex> lock(cv_mutex);
            is_connected = true;
        }
    }
    cv.notify_all(); // 通知所有等待的线程
}

// 构造函数，初始化Zookeeper句柄为nullptr
ZKclient::ZKclient() : m_zhandle(nullptr) {}

// 构造函数，初始化Zookeeper句柄为nullptr
ZKclient::~ZKclient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}

// zk客户端启动zk服务端
void ZKclient::Start()
{
    std::string host = RPCapplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = RPCapplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string conn_str = host + ":" + port;
    /**
     * zookeeper_mt: 多线程版本,
     * zookeeper的API客户端程序提供了三个线程：
     * API调用线程、
     * 网络I/O线程（pthread_create, poll）、
     * watcher回调线程（pthread_create）
     */
    // zookeeper_init函数初始化zk对象，异步建立RPC server和ZK client之间的连接
    m_zhandle = zookeeper_init(conn_str.c_str(), global_watcher, 6000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        LOG(ERROR) << "zookeeper_init error";
        exit(EXIT_FAILURE);
    }

    std::unique_lock<std::mutex> lock(cv_mutex); // 锁定全局锁
    cv.wait(lock, []
            { return is_connected; });     // 等待连接状态变为已连接
    LOG(INFO) << "zookeeper_init success"; // 日志记录连接成功信息
}

// 在zk服务端中依据参数值(路径、数据、数据长度、状态[默认为0])创建一个节点
void ZKclient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE)
    {
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            LOG(INFO) << "znode create success... path:" << path;
        }
        else
        {
            LOG(ERROR) << "znode create fail... path:" << path;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据指定的znode节点路径，来获取znode节点值
std::string ZKclient::GetData(const char *path)
{
    char buf[64];
    int bufferlen = sizeof(buf);
    int flag = zoo_get(m_zhandle, path, 0, buf, &bufferlen, nullptr);
    if (flag != ZOK)
    {
        LOG(ERROR) << "zoo_get error";
        return "";
    }
    else
    {
        return buf;
    }
}