#ifndef INCLUDE_OFFLINEMSGMODEL_H_
#define INCLUDE_OFFLINEMSGMODEL_H_

#include <string>
#include <vector>

using namespace std;

class OfflineMsgModel {
 public:
    void insert(int userid, string msg);
    void remove(int userid);
    vector<string> query(int userid);
};
#endif  // INCLUDE_OFFLINEMSGMODEL_H_
