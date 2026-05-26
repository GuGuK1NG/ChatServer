#include "chatserver.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "chatservice.hpp"
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册函数回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 用户断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // json反序列化
    json js = json::parse(buf);
    // 通过js["msgid"] 获取一个业务handler来回调处理函数
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);
}
