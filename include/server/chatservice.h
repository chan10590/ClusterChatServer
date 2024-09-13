#ifndef INCLUDE_CHATSERVICE_H_
#define INCLUDE_CHATSERVICE_H_

#include <muduo/net/TcpServer.h>
#include <functional>
#include <json.hpp>
#include <mutex>
#include <unordered_map>
#include "friendmodel.h"
#include "groupmodel.h"
#include "offlinemsgmodel.h"
#include "redis.h"
#include "usermodel.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp time)>;

class ChatService {
 public:
    static ChatService* instance();

    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void grouChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void logout(const TcpConnectionPtr& conn, json& js, Timestamp time);
    MsgHandler getHandler(int msgid);

    void clientExceptClose(const TcpConnectionPtr& conn);
    void reset();

    void handleRedisSubscribeMsg(int channel, string message);

 private:
    ChatService();
    unordered_map<int, MsgHandler> _msg_handlers;
    // 根据用户id找到连接
    unordered_map<int, TcpConnectionPtr> _user_connections;
    mutex _user_conn_mtx;
    UserModel _user_model;
    OfflineMsgModel _offline_msg_model;
    FriendModel _friend_model;
    GroupModel _group_model;
    Redis _redisQ;
};
#endif  // INCLUDE_CHATSERVICE_H_
