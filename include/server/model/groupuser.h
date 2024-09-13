#ifndef INCLUDE_GROUPUSER_H_
#define INCLUDE_GROUPUSER_H_

#include "user.h"

class GroupUser : public User {
 public:
    void setRole(string role) { _role = role; }
    string getRole() { return _role; }

 private:
    string _role;
};
#endif  // INCLUDE_GROUPUSER_H_
