#include "chatservice.h"
#include <muduo/base/Logging.h>
#include "util.h"

using namespace placeholders;
using namespace muduo;

ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

ChatService::ChatService() {
    _msg_handlers.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msg_handlers.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msg_handlers.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msg_handlers.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    _msg_handlers.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msg_handlers.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msg_handlers.insert({GROUP_CHAT_MSG, std::bind(&ChatService::grouChat, this, _1, _2, _3)});
    _msg_handlers.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});

    if (_redisQ.connect()) {
        _redisQ.setNotifyHandler(std::bind(&ChatService::handleRedisSubscribeMsg, this, _1, _2));
    }
}

MsgHandler ChatService::getHandler(int msgid) {
    auto it = _msg_handlers.find(msgid);
    if (it == _msg_handlers.end()) {
        // 返回空处理器，既可以打印日志信息，又不会因无处理器退出程序
        return [msgid](const TcpConnectionPtr& conn, json& js, Timestamp time) {
            LOG_ERROR << "message id " << msgid << " can not find according handler!";
        };
    } else {
        return _msg_handlers[msgid];
    }
}

void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _user_model.query(id);
    if (user.getId() != -1 && pwd == user.getPassword()) {
        if (user.getState() == "online") {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "user has been logined";
            conn->send(response.dump());
        } else {
            {
                lock_guard<mutex> lock(_user_conn_mtx);
                _user_connections.insert({id, conn});
            }
            LOG_INFO << "log success";
            _redisQ.subscribe(id);  // 向redisQ订阅channel
            LOG_INFO << "subscribe channel " << id;
            user.setState("online");
            _user_model.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            LOG_INFO << "add base info";

            vector<string> vec = _offline_msg_model.query(id);
            if (!vec.empty()) {
                response["offlinemsg"] = vec;
                _offline_msg_model.remove(id);
            }
            LOG_INFO << "add offline msg";

            vector<User> friend_vec = _friend_model.query(id);
            if (!friend_vec.empty()) {
                vector<string> vectmp;
                for (User& f : friend_vec) {
                    json js1;
                    js1["id"] = f.getId();
                    js1["name"] = f.getName();
                    js1["state"] = f.getState();
                    vectmp.push_back(js1.dump());
                }
                response["friends"] = vectmp;
            }
            LOG_INFO << "add friend";

            vector<Group> group_vec = _group_model.queryGroups(id);
            if (!group_vec.empty()) {
                vector<string> vectmp1;
                for (Group& g : group_vec) {
                    json jsg;
                    jsg["id"] = g.getId();
                    jsg["groupname"] = g.getName();
                    jsg["groupdesc"] = g.getDesc();
                    vector<string> uservec;
                    for (GroupUser& gu : g.getUsers()) {
                        json jsgu;
                        jsgu["id"] = gu.getId();
                        jsgu["name"] = gu.getName();
                        jsgu["state"] = gu.getState();
                        jsgu["role"] = gu.getRole();
                        uservec.push_back(jsgu.dump());
                    }
                    jsg["groupusers"] = uservec;
                    vectmp1.push_back(jsg.dump());
                }
                response["groups"] = vectmp1;
            }
            LOG_INFO << "add group";
            conn->send(response.dump());
            LOG_INFO << "sen response success";
        }
    } else {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "user name or password error";
        conn->send(response.dump());
    }
}
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool reg_state = _user_model.insert(user);
    if (reg_state) {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    } else {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int peer_id = js["to"].get<int>();

    {
        lock_guard<mutex> lock(_user_conn_mtx);
        auto it = _user_connections.find(peer_id);
        // 用户在线
        if (it != _user_connections.end()) {
            it->second->send(js.dump());
            return;
        }
    }
    User user = _user_model.query(peer_id);
    if (user.getState() == "online") {
        _redisQ.publish(peer_id, js.dump());
        return;
    }
    // 用户离线消息
    _offline_msg_model.insert(peer_id, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friend_model.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1, name, desc);
    if (_group_model.createGroup(group)) {
        _group_model.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _group_model.addGroup(userid, groupid, "normal");
}

void ChatService::grouChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> userid_vec = _group_model.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_user_conn_mtx);
    for (int id : userid_vec) {
        auto it = _user_connections.find(id);
        // 用户在线
        if (it != _user_connections.end()) {
            it->second->send(js.dump());
        } else {
            User user = _user_model.query(id);
            if (user.getState() == "online") {
                _redisQ.publish(id, js.dump());
                return;
            } else {
                // 用户离线消息
                _offline_msg_model.insert(id, js.dump());
            }
        }
    }
}
void ChatService::logout(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_user_conn_mtx);
        auto it = _user_connections.find(userid);
        if (it != _user_connections.end()) {
            _user_connections.erase(it);
        }
    }
    _redisQ.unsubscribe(userid);  // 用户下线取消订阅

    User user(userid, "", "", "offline");
    _user_model.updateState(user);
}
void ChatService::clientExceptClose(const TcpConnectionPtr& conn) {
    User user;
    {
        lock_guard<mutex> lock(_user_conn_mtx);
        for (auto it = _user_connections.begin(); it != _user_connections.end(); ++it) {
            if (it->second == conn) {
                user.setId(it->first);
                _user_connections.erase(it);
                break;
            }
        }
    }

    _redisQ.unsubscribe(user.getId());  // 用户异常退出取消订阅
    if (user.getId() != -1) {
        user.setState("offline");
        _user_model.updateState(user);
    }
}

void ChatService::reset() { _user_model.resetState(); }

void ChatService::handleRedisSubscribeMsg(int userid, string msg) {
    json js = json::parse(msg.c_str());

    lock_guard<mutex> lock(_user_conn_mtx);
    auto it = _user_connections.find(userid);
    if (it != _user_connections.end()) {
        it->second->send(js.dump());
        return;
    }
    _offline_msg_model.insert(userid, js.dump());
}
