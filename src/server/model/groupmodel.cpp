#include "groupmodel.h"
#include "db.h"
#include "groupuser.h"

bool GroupModel::createGroup(Group& group) {
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userid, int groupid, string role) {
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser(groupid, userid, grouprole) values(%d, %d, '%s')", groupid,
            userid, role.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userid) {
    char sql[1024] = {0};
    sprintf(sql,
            "select g.id,g.groupname,g.groupdesc from AllGroup g inner join GroupUser u \
            on g.id=u.groupid where u.userid = %d",
            userid);

    vector<Group> groupvec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupvec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    for (Group& group : groupvec) {
        sprintf(sql,
                "select u.id,u.name,u.state,gu.grouprole from User u inner join GroupUser gu \
                 on u.id=gu.userid where gu.groupid = %d",
                group.getId());
        MYSQL_RES* res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                GroupUser groupuser;
                groupuser.setId(atoi(row[0]));
                groupuser.setName(row[1]);
                groupuser.setState(row[2]);
                groupuser.setRole(row[3]);
                group.getUsers().push_back(groupuser);
            }
            mysql_free_result(res);
        }
    }

    return groupvec;
}
vector<int> GroupModel::queryGroupUsers(int userid, int groupid) {
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", groupid,
            userid);

    vector<int> idvec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                idvec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idvec;
}
