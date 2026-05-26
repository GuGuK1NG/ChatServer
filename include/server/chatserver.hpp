#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);

    // 启动
    void start();

private:
    // 回调
    void onConnection(const TcpConnectionPtr &);
    void onMessage(const TcpConnectionPtr &,
                   Buffer *,
                   Timestamp);
    TcpServer _server;
    EventLoop *_loop;
};