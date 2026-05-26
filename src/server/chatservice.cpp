#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
#include <map>
using namespace std;
ChatService *ChatService::instance()
{

    static ChatService service;
    return &service;
}
// 注册消息以及回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, [this](const TcpConnectionPtr &conn,
                                             json &js, Timestamp time)
                           {
                               this->login(conn, js, time);
                           }});
    _msgHandlerMap.insert({REG_MSG, [this](const TcpConnectionPtr &conn,
                                           json &js, Timestamp time)
                           {
                               this->reg(conn, js, time);
                           }});
    _msgHandlerMap.insert({PRIVATE_CHAT_MSG, [this](const TcpConnectionPtr &conn,
                                                    json &js, Timestamp time)
                           {
                               this->privatechat(conn, js, time);
                           }});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
                           {
                               this->addFriend(conn, js, time);
                           }});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
                           {
                               this->createGroup(conn, js, time);
                           }});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
                           {
                               this->groupChat(conn, js, time);
                           }});
    _msgHandlerMap.insert({ADD_GROUP_MSG, [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
                           {
                               this->joinGroup(conn, js, time);
                           }});
    _msgHandlerMap.insert({LOGOUT_MSG, [this](const TcpConnectionPtr &conn, json &js, Timestamp time)
                           {
                               this->logout(conn, js, time);
                           }});

    if (_redis.connect())
    {
        _redis.init_notify_handler(
            [this](const int &channel, const string &message)
            { this->handleRedisSubscribeMessage(channel, message); }

        );
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{

    string name = js["name"];
    string pwd = js["password"];

    User user = _userModel.query(name);
    int id = user.getId();
    if (user.getId() != -1 && user.getPwd() == pwd)
    {
        json response;
        if (user.getState() == "online")
        {
            // 禁止重复登录
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账户已经登录";
            conn->send(response.dump());
        }

        else
        {
            // success
            {
                std::shared_lock<std::shared_mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
                _connUserMap.insert({conn, id});
            }

            // Redis订阅
            _redis.subscribe(id);

            // 在线状态更改
            user.setState("online");
            _userModel.updateState(user);

            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取后删除
                _offlineMsgModel.remove(id);
            }
            // 查询用户好友信息
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            vector<Group> groupvec = _groupModel.queryGroups(user.getId());
            if (!groupvec.empty())
            {
                vector<string> vec2;
                for (Group &group : groupvec)
                {
                    json js;
                    js["groupid"] = group.getId();
                    js["groupname"] = group.getName();
                    js["groupdesc"] = group.getDesc();
                    vec2.push_back(js.dump());
                }
                response["groups"] = vec2;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或者密码错误";
        conn->send(response.dump());
    }
}
// name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // success
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["name"] = name;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["name"] = name;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
}
// 服务器异常
void ChatService::reset()
{
    _userModel.resetState();
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {

        return [=](const TcpConnectionPtr &conn,
                   json &js, Timestamp time)
        {
            LOG_ERROR << "Msgid: " << msgid << " can not find handler";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    int userId = -1;
    {
        std::shared_lock<std::shared_mutex> lock(_connMutex);
        auto it = _connUserMap.find(conn);
        if (it != _connUserMap.end())
        {
            userId = it->second;
            _userConnMap.erase(userId);
            _connUserMap.erase(it);
        }
    }

    _redis.unsubscribe(userId);

    // 更新用户状态信息
    if (userId != -1)
    {
        user.setId(userId);
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::privatechat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    TcpConnectionPtr conn_ptr;

    {
        std::shared_lock<shared_mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // 在此服务器在线
            conn_ptr = it->second;
        }
    }
    if (conn_ptr && conn_ptr->connected())
    {

        conn_ptr->send(js.dump());
        return;
    }
    // 查询是否在其他服务器在线
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // 不在线,存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
    return;
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    _friendModel.insert(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}
//
void ChatService::joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    vector<TcpConnectionPtr> onlineusers;
    vector<int> pendingusers;
    string message = js.dump();
    {
        shared_lock<shared_mutex> lock(_connMutex);
        for (int id : useridVec)
        {
            if (id == userid)
            {
            }
            auto it = _userConnMap.find(id);
            if (it != _userConnMap.end())
            {
                onlineusers.push_back(it->second);
            }
            else
            {
                pendingusers.push_back(id);
            }
        }
    }
    for (auto &onlineuser : onlineusers)
    {
        if (onlineuser && onlineuser->connected())
        {
            onlineuser->send(message);
        }
    }
    for (int id : pendingusers)
    {
        User user = _userModel.query(id);
        if (user.getState() == "online")
        {
            _redis.publish(id, message);
        }
        else
        {
            _offlineMsgModel.insert(id, message);
        }
    }
}

void ChatService::logout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        shared_lock<shared_mutex> lock(_connMutex);
        _userConnMap.erase(userid);
        _connUserMap.erase(conn);
    }
    User user(userid);
    _redis.unsubscribe(userid);
    _userModel.updateState(user);
}

void ChatService::handleRedisSubscribeMessage(int channel, string message)
{
    TcpConnectionPtr conn;
    {
        shared_lock<shared_mutex> lock(_connMutex);
        auto it = _userConnMap.find(channel);
        if (it != _userConnMap.end())
        {
            conn = it->second;
        }
    }
    if (conn && conn->connected())
    {
        conn->send(message);
    }
    else
    {
        _offlineMsgModel.insert(channel, message);
    }
}
