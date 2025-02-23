#include "db.h"
#include <muduo/base/Logging.h>

static string server = "127.0.0.1";
static string user = "cpp";
static string password = "88888888";
static string dbname = "chat";

MySQL::MySQL() : _conn(mysql_init(nullptr)) {}

MySQL::~MySQL() {
    if (_conn) {
        mysql_close(_conn);
    }
}

bool MySQL::connect() {
    MYSQL* p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(),
                                  dbname.c_str(), 3306, nullptr, 0);
    if (p) {
        // 防止mysql中文乱码
        mysql_query(_conn, "set name gbk");
    }
    return p;
}

bool MySQL::update(string sql) {
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << " update fail!";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(string sql) {
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << " query fail!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL* MySQL::getConnection() { return _conn; }
