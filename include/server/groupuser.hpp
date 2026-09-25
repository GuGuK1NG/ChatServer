#pragma once

#include "user.hpp"

class GroupUser : public User
{
public:
    // 设置个组内管理员
    void setRole(string role)
    {
        this->role = role;
    };
    string &getRole()
    {
        return this->role;
    }

private:
    string role;
};