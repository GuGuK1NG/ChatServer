#pragma once

#include "usermodel.hpp"
#include "offlinemessage.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <mutex>
#include <shared_mutex>
using json = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;
// 表示处理消息的时间回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
// 业务类
// 单列
class ChatService
{
public:
    static ChatService *instance();
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 私聊
    void privatechat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组
    void joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群组聊天
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 客户端正常退出
    void logout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 服务器异常处理方法
    void reset();
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    void handleRedisSubscribeMessage(int channel, string message);

private:
    ChatService();
    // 消息id和对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 双映射
    unordered_map<TcpConnectionPtr, int> _connUserMap;
    //_userConnMap的互斥锁
    std::shared_mutex _connMutex;

    //  数据操作类对象
    UserModel _userModel;
    OfflineMessgaeModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    Redis _redis;
};