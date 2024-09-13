#ifndef INCLUDE_GROUPMODEL_H_
#define INCLUDE_GROUPMODEL_H_

#include "group.h"

// group相关表AllGroup、GroupUser的数据操作类
class GroupModel {
 public:
    bool createGroup(Group& group);
    void addGroup(int userid, int groupid, string role);
    vector<Group> queryGroups(int userid);
    vector<int> queryGroupUsers(int userid, int groupid);
};
#endif  // INCLUDE_GROUPMODEL_H_
