#pragma once

#include "gourp.hpp"
#include <string>
#include <vector>

using namespace std;

class GroupModel
{
public:
    bool createGroup(Group &group);
    void addGroup(int userid, int groupid, string role);
    vector<Group> queryGroups(int userid);
    vector<int> queryGroupUsers(int userid, int groupid);
};