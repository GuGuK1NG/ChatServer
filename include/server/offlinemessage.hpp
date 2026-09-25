#pragma once

#include <string>
#include <vector>
using namespace std;
// 提供离线消息表的操作接口方法
class OfflineMessgaeModel
{
public:
    // 存储用户的离线消息
    void insert(int userid, string message);
    // 删除
    void remove(int userid);
    // 查询
    vector<string> query(int userid);

private:
};