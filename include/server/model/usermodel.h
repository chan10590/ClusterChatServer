#ifndef INCLUDE_USERMODEL_H_
#define INCLUDE_USERMODEL_H_

#include "user.h"

// User表的数据操作类
class UserModel {
 public:
    bool insert(User& user);
    User query(int id);
    bool updateState(User user);
    void resetState();
};
#endif  // INCLUDE_USERMODEL_H_
