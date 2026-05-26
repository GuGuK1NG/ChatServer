#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <atomic>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "gourp.hpp"
#include "user.hpp"
#include "public.hpp"

User g_currentUser;
vector<User> g_currentUserFriendList;
vector<Group> g_currentUserGroupList;

// 接收线程
void readTaskHandler(int clientfd);

// 获取系统时间
string getCurrentTime();
// command handler
void help(int fd = 0, string str = "");
void chat(int, string);
void addfriend(int, string);
void creategroup(int, string);
void addgroup(int, string);
void groupchat(int, string);
void logout(int, string);

void doLoginResponse(const json &js);       // 处理登录响应
void doRegResponse(const json &responsejs); // 处理注册响应

unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"logout", "注销,格式logout"}};
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"logout", logout}};

bool isMainMenuRunning = false;
// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic<bool> g_isLoggedIn{false};
void mainMenu(int clientfd);
void showCurrentUserData();

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid!" << endl;
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket create error" << endl;
        exit(EXIT_FAILURE);
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(EXIT_FAILURE);
    }
    // 初始化读写线程通信的信号量
    sem_init(&rwsem, 0, 0);

    std::thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    for (;;)
    {
        cout << "====================" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        cout << "====================" << endl;
        int choice = 0;
        cin >> choice;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            // cin.get();
        }

        switch (choice)
        {
        case 1:
        {

            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin >> name;
            cin.get();
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            g_isLoggedIn.store(false);

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send login msg error:" << request << endl;
            }

            sem_wait(&rwsem); // 等待登录结果

            if (g_isLoggedIn.load())
            {

                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
            else
            {
                cerr << "login failed, please try again!" << endl;
            }
            break;
        }
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50);
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);

            if (len == -1)
            {
                cerr << "send,reg msg error:" << request << endl;
                cerr << "errno: " << errno << " - " << strerror(errno) << endl;
            }

            sem_wait(&rwsem); // 等待注册结果
            break;
        }
        case 3:
        {
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
            break;
        }
        default:
        {
            cerr << "invalid input!" << endl;

            break;
        }
        }
    }
}

// 处理登录响应
void doLoginResponse(const json &responsejs)
{

    if (0 != responsejs["errno"].get<int>())
    {
        cerr << responsejs["errmsg"] << endl;
        g_isLoggedIn.store(false);
    }
    else
    {
        cout << "current user set" << endl;
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        if (responsejs.contains("friends"))
        {
            g_currentUserFriendList.clear();
            vector<string> vec = responsejs["friends"];

            for (string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }
        if (responsejs.contains("groups"))
        {
            g_currentUserGroupList.clear();
            vector<string> vec1 = responsejs["groups"];
            for (string &str : vec1)
            {
                json js = json::parse(str);
                Group group;
                group.setId(js["groupid"].get<int>());
                group.setName(js["groupname"]);
                group.setDesc(js["groupdesc"]);
                g_currentUserGroupList.push_back(group);
            }
        }
        showCurrentUserData();

        if (responsejs.contains("offlinemsg"))
        {
            vector<string> vec = responsejs["offlinemsg"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                int msgtype = js["msgid"].get<int>();
                if (PRIVATE_CHAT_MSG == msgtype)
                {
                    cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"]
                         << " said:" << js["msg"].get<string>() << endl;
                }
                else if (GROUP_CHAT_MSG == msgtype)
                {
                    cout << "群消息[" << js["groupname"] << "]:" << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"]
                         << " said:" << js["msg"].get<string>() << endl;
                }
            }
        }
        g_isLoggedIn.store(true);
    }
}
// 处理注册响应
void doRegResponse(const json &responsejs)
{

    if (0 != responsejs["errno"].get<int>())
    {
        cerr << responsejs["name"] << " is already exist, register error!" << endl;
    }
    else
    {
        cout << responsejs["name"] << " register success,userid is " << responsejs["id"] << endl;
    }
}

// 接收线程
void readTaskHandler(int clientfd)
{
    while (true)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (-1 == len || 0 == len)
        {
            close(clientfd);
            exit(-1);
        }
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if (PRIVATE_CHAT_MSG == msgtype)
        {
            cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"]
                 << " said:" << js["msg"].get<string>() << endl;
            continue;
        }
        else if (GROUP_CHAT_MSG == msgtype)
        {
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"]
                 << " said:" << js["msg"].get<string>() << endl;
            continue;
        }
        else if (LOGIN_MSG_ACK == msgtype)
        {
            doLoginResponse(js); // 处理登录响应
            sem_post(&rwsem);    // 通知主线程登录结果
            continue;
        }
        else if (REG_MSG_ACK == msgtype)
        {
            doRegResponse(js); // 处理注册响应
            sem_post(&rwsem);  // 通知主线程注册结果
            continue;
        }
    }
    return;
}

void showCurrentUserData()
{
    cout << "=================Login User=================" << endl;
    cout << "current login user =>id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "=================Friend List================" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    else
    {
        cout << "EMPTY" << endl;
    }
    cout << "=================Group List==================" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " "
                     << user.getRole() << endl;
            }
        }
    }
    else
    {
        cout << "EMPTY" << endl;
    }
    cout << "=============================================" << endl;
}

void mainMenu(int clientfd)
{
    help();
    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command" << endl;
            continue;
        }
        it->second(clientfd,
                   commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

void help(int fd, string str)
{
    cout << "show command list >>>" << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << ":" << p.second << endl;
    }
    cout << endl;
}
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }
    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = PRIVATE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error ->" << buffer << endl;
    }
}
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(),
                   strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error ->" << buffer << endl;
    }
}
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error ->" << buffer << endl;
    }
}
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(),
                   strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error ->" << buffer << endl;
    }
}
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }
    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error ->" << buffer << endl;
    }
}
void logout(int clientfd, string str)
{

    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(),
                   strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send logout msg error ->" << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
        g_isLoggedIn.store(false);
        shutdown(clientfd, SHUT_RDWR);
        cout << "logout success!" << endl;
    }
}

string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;

    localtime_r(&time_now, &tm_now);

    std::ostringstream oss;
    oss << std::setfill('0')
        << (tm_now.tm_year + 1900) << "-"
        << std::setw(2) << (tm_now.tm_mon + 1) << "-"
        << std::setw(2) << tm_now.tm_mday << " "
        << std::setw(2) << tm_now.tm_hour << ":"
        << std::setw(2) << tm_now.tm_min << ":"
        << std::setw(2) << tm_now.tm_sec;
    return oss.str();
}