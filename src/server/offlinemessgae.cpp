#include "offlinemessage.hpp"
#include "mysql.h"
// 存储用户的离线消息
void OfflineMessgaeModel::insert(int userid, string message)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values('%d','%s')", userid, message.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 删除
void OfflineMessgaeModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 查询
vector<string> OfflineMessgaeModel::query(int userid)
{
    char sql[128] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = '%d'", userid);
    MySQL mysql;
    vector<string> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 把所有离线消息放入vector中
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }

            mysql_free_result(res);
        }
    }
    return vec;
}