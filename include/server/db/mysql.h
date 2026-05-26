#ifndef MYSQL_H
#define MYSQL_H

#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <string>
using namespace std;

static string server = "127.0.0.1";
static string user = "root";
static string password = "12345678";
static string dbname = "chat";

class MySQL
{
public:
    MySQL();
    ~MySQL();

    bool connect();
    bool update(string sql);
    // 查询操作
    MYSQL_RES *query(string sql);
    // 获取连接
    MYSQL *getConnection();

private:
    MYSQL *_conn;
};
#endif