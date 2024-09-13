#ifndef INCLUDE_GROUP_H_
#define INCLUDE_GROUP_H_

#include <string>
#include <vector>
#include "groupuser.h"

using namespace std;

// AllGroup表的ORM类
class Group {
 public:
    Group(int id = -1, string name = "", string desc = "") : _id(id), _name(name), _desc(desc) {}

    void setId(int id) { _id = id; }
    void setName(string name) { _name = name; }
    void setDesc(string desc) { _desc = desc; }

    int getId() { return _id; }
    string getName() { return _name; }
    string getDesc() { return _desc; }
    vector<GroupUser>& getUsers() { return _users; }

 private:
    int _id;
    string _name;
    string _desc;
    vector<GroupUser> _users;
};

#endif  // INCLUDE_GROUP_H_
