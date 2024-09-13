#ifndef INCLUDE_FRIENDMODEL_H_
#define INCLUDE_FRIENDMODEL_H_

#include <user.h>
#include <vector>

using namespace std;

class FriendModel {
 public:
    void insert(int userid, int friendid);
    vector<User> query(int userid);
};

#endif  // INCLUDE_FRIENDMODEL_H_
