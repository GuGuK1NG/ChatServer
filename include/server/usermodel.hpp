#pragma once

#include "user.hpp"

class UserModel
{
public:
    bool insert(User &user);

    User query(string &name);
    User query(int &id);

    // 更新在线状态
    bool updateState(User &user);

    void resetState();

private:
};