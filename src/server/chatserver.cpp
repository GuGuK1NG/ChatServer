#include "chatserver.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "chatservice.hpp"
#include <functional>
#include <string>
#include <cstring>
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
    while(true){

        const char* data=buffer->peek();//获取缓冲区可读数据的起始地址
        size_t len=buffer->readableBytes(); //可读字节数
        if(len==0){
            break;
        }
        // 在已收到的字节里找分隔符 '\0'
        const void *found = memchr(data, '\0', len);
        if (found == nullptr){
            break;                                // 未收全（拆包），等下次回调
        }
        size_t msglen = static_cast<const char *>(found) - data; // 消息长度
        string buf(data, msglen); // 取出一条完整的消息
        buffer->retrieve(msglen + 1); // 从缓冲区移除已处理,包括分隔符 '\0'
        if(buf.empty()){
            continue;      //空消息跳过
        }

        // json反序列化
        json js = json::parse(buf);
        // 通过js["msgid"] 获取一个业务handler来回调处理函数
        auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
        msgHandler(conn, js, time);
    }
    string buf = buffer->retrieveAllAsString();
    
}
