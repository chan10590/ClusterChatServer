#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "group.h"
#include "user.h"
#include "util.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

User g_current_user;
vector<User> g_friend_list;
vector<Group> g_group_list;

void showCurrentUserInfo();
void taskHandler(int fd);
string getCurrentTime();
void chatMenu(int fd);

int main(int argc, char* argv[]) {
    cout << "===available nginx proxy:" << endl;
    cout << "   127.0.0.1:9000" << endl;
    if (argc < 3) {
        cerr << "invalid command! format: ./Client 127.0.0.1 9000" << endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = inet_addr(ip);
    connect(fd, (sockaddr*)&serv, sizeof(serv));

    while (1) {
        cout << "===================Menu======================" << endl;
        cout << "*** select ***            *** description ***" << endl;
        cout << "       1                         login       " << endl;
        cout << "       2                        register     " << endl;
        cout << "       3                          quit       " << endl;
        cout << "=============================================" << endl;

        cout << "select ";
        int selector = 0;
        cin >> selector;
        cin.get();
        switch (selector) {
            case 1: {
                cout << "===================Login======================" << endl;
                int id = 0;
                char pwd[50] = {0};
                cout << "userid: ";
                cin >> id;
                cin.get();
                cout << "password: ";
                cin.getline(pwd, 50);
                cout << endl;

                json request_jsobj;
                request_jsobj["msgid"] = LOGIN_MSG;
                request_jsobj["id"] = id;
                request_jsobj["password"] = pwd;
                string request_jsstr = request_jsobj.dump();
                cout << "request dump" << endl;
                int ret = send(fd, request_jsstr.c_str(), strlen(request_jsstr.c_str()) + 1, 0);
                cout << "send request" << endl;
                if (ret > 0) {
                    char buf[1024] = {0};
                    int ret1 = recv(fd, buf, 1024, 0);
                    cout << "recv response" << endl;
                    if (ret1 == -1) {
                        cerr << "recv fail" << endl;
                    } else {
                        json response_jsobj = json::parse(buf);
                        cout << "parse response json str" << endl;
                        if (response_jsobj["errno"].get<int>()) {
                            cerr << "login fail" << endl;
                        } else {
                            g_current_user.setId(response_jsobj["id"].get<int>());
                            g_current_user.setName(response_jsobj["name"]);

                            if (response_jsobj.contains("friends")) {
                                vector<string> vec = response_jsobj["friends"];
                                for (string& str : vec) {
                                    json jsobj = json::parse(str);
                                    User user;
                                    user.setId(jsobj["id"].get<int>());
                                    user.setName(jsobj["name"]);
                                    user.setState(jsobj["state"]);
                                    g_friend_list.push_back(user);
                                }
                            }

                            if (response_jsobj.contains("friends")) {
                                vector<string> vec = response_jsobj["friends"];
                                for (string& str : vec) {
                                    json jsobj = json::parse(str);
                                    User user;
                                    user.setId(jsobj["id"].get<int>());
                                    user.setName(jsobj["name"]);
                                    user.setState(jsobj["state"]);
                                    g_friend_list.push_back(user);
                                }
                            }

                            if (response_jsobj.contains("groups")) {
                                vector<string> vec = response_jsobj["groups"];
                                for (string& str : vec) {
                                    json jsobj = json::parse(str);
                                    Group group;
                                    group.setId(jsobj["id"].get<int>());
                                    group.setName(jsobj["groupname"]);
                                    group.setDesc(jsobj["groupdesc"]);
                                    vector<string> vec1 = response_jsobj["groupusers"];
                                    for (string& str1 : vec1) {
                                        GroupUser gu;
                                        json jsobj1 = json::parse(str1);
                                        gu.setId(jsobj1["id"].get<int>());
                                        gu.setName(jsobj1["name"]);
                                        gu.setState(jsobj1["state"]);
                                        gu.setRole(jsobj1["role"]);
                                        group.getUsers().push_back(gu);
                                    }
                                    g_group_list.push_back(group);
                                }
                            }
                            cout << "get user info" << endl;
                            showCurrentUserInfo();

                            cout << "==========offlinemessage" << endl;
                            if (response_jsobj.contains("offlinemsg")) {
                                vector<string> vec = response_jsobj["offlinemsg"];
                                for (string& str : vec) {
                                    json jsobj = json::parse(str);
                                    int msg_type = jsobj["msgid"].get<int>();
                                    if (msg_type == ONE_CHAT_MSG) {
                                        cout << "   " << jsobj["time"] << " [" << jsobj["id"] << "] " << jsobj["name"]
                                             << "send: " << jsobj["msg"] << endl;
                                        continue;
                                    } else if (msg_type == GROUP_CHAT_MSG) {
                                        cout << "   Group " << jsobj["groupid"] << " " << jsobj["time"] << " ["
                                             << jsobj["id"] << "] " << jsobj["name"] << ": " << jsobj["msg"] << endl;
                                    }
                                }
                            }
                            static int read_thdnum = 0;
                            if (read_thdnum == 0) {
                                std::thread read_task(taskHandler, fd);
                                read_task.detach();
                                read_thdnum++;
                            }
                            chatMenu(fd);
                        }
                    }
                } else if (ret == 0) {
                    cout << "server close" << endl;
                } else {
                    cerr << "send fail" << endl;
                }
            }
                cout << "==============================================" << endl;
                break;
            case 2: {
                cout << "==================Register====================" << endl;
                char name[50] = {0};
                char pwd[50] = {0};
                cout << "username: ";
                cin.getline(name, 50);
                cout << "password: ";
                cin.getline(pwd, 50);
                cout << endl;

                json request_jsobj;
                request_jsobj["msgid"] = REG_MSG;
                request_jsobj["name"] = name;
                request_jsobj["password"] = pwd;
                string request_jsstr = request_jsobj.dump();

                int ret = send(fd, request_jsstr.c_str(), strlen(request_jsstr.c_str()) + 1, 0);
                if (ret > 0) {
                    char buf[1024] = {0};
                    int ret1 = recv(fd, buf, 1024, 0);
                    if (ret1 == -1) {
                        cerr << "recv fail" << endl;
                    } else {
                        json response_jsobj = json::parse(buf);
                        if (response_jsobj["errno"].get<int>()) {
                            cerr << "register fail" << endl;
                        } else {
                            cout << name << " register success! userid is " << response_jsobj["id"] << endl;
                        }
                    }
                } else if (ret == 0) {
                    cout << "server close" << endl;
                } else {
                    cerr << "send fail" << endl;
                }
            }
                cout << "==============================================" << endl;
                break;
            case 3:
                close(fd);
                exit(0);
            default:
                break;
        }
    }

    return 0;
}

void showCurrentUserInfo() {
    cout << "user(id: " << g_current_user.getId() << ", name: " << g_current_user.getName() << ") login!" << endl;
    cout << "==========friend list" << endl;
    if (!g_friend_list.empty()) {
        for (User& u : g_friend_list) {
            cout << "   " << u.getId() << " " << u.getName() << " " << u.getState() << endl;
        }
    }
    cout << "=====================" << endl;
    cout << "==========group list" << endl;
    if (!g_group_list.empty()) {
        for (Group& g : g_group_list) {
            cout << "=== group" << endl;
            cout << "   " << g.getId() << " " << g.getName() << " " << g.getDesc() << endl;
            cout << "=== group user" << endl;
            for (GroupUser& gu : g.getUsers()) {
                cout << "   " << gu.getId() << " " << gu.getName() << " " << gu.getRole() << " " << gu.getState()
                     << endl;
            }
        }
    }
    cout << "====================" << endl;
}
// 任务处理函数，用于接收服务器发来的消息
void taskHandler(int fd) {
    while (1) {
        char buf[1024] = {0};
        int ret = recv(fd, buf, 1024, 0);
        if (ret == -1 || ret == 0) {
            close(fd);
            exit(-1);
        }
        json jsobj = json::parse(buf);
        int msg_type = jsobj["msgid"].get<int>();
        if (msg_type == ONE_CHAT_MSG) {
            cout << jsobj["time"] << " [" << jsobj["id"] << "] " << jsobj["name"] << "send: " << jsobj["msg"] << endl;
            continue;
        } else if (msg_type == GROUP_CHAT_MSG) {
            cout << "Group " << jsobj["groupid"] << " " << jsobj["time"] << " [" << jsobj["id"] << "] " << jsobj["name"]
                 << ": " << jsobj["msg"] << endl;
        }
    }
}
// 获取当前时间的字符串表示
string getCurrentTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[80];
    sprintf(buf, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(buf);
}

void help(int fd = 0, string str = "");
void oneChat(int, string);
void addFriend(int, string);
void createGroup(int, string);
void addGroup(int, string);
void groupChat(int, string);
void logout(int, string);

unordered_map<string, string> cmdMap = {{"help", "help"},
                                        {"chat", "One-to-one chat, chat:friendid:message"},
                                        {"addfriend", "Add a friend, addfriend:friendid"},
                                        {"creategroup", "Create a group, creategroup:groupname:groupdesc"},
                                        {"addgroup", "Join a group, addgroup:groupid"},
                                        {"groupchat", "Group chat, groupchat:groupid:message"},
                                        {"logout", "logout"}};
unordered_map<string, function<void(int, string)>> cmdHandlerMap = {
    {"help", help},         {"chat", oneChat},        {"addfriend", addFriend}, {"creategroup", createGroup},
    {"addgroup", addGroup}, {"groupchat", groupChat}, {"logout", logout}};

void chatMenu(int fd) {
    help();

    char buf[1024] = {0};
    while (1) {
        cin.getline(buf, 1024);
        string cmdbuf(buf);
        string cmd;
        int idx = cmdbuf.find(":");
        if (idx == -1) {
            cmd = cmdbuf;
        } else {
            cmd = cmdbuf.substr(0, idx);
        }
        auto it = cmdHandlerMap.find(cmd);
        if (it == cmdHandlerMap.end()) {
            cerr << "invalid command!" << endl;
            continue;
        }
        it->second(fd, cmdbuf.substr(idx + 1, cmdbuf.size() - idx));
    }
}

void help(int fd, string str) {
    cout << "===================Command List===================" << endl;
    for (auto& cmd : cmdMap) {
        cout << cmd.first << " : " << cmd.second << endl;
    }
    cout << "==================================================" << endl;
}

void oneChat(int fd, string str) {
    int idx = str.find(":");
    int friendid = stoi(str.substr(0, idx));
    string message = str.substr(idx + 1, str.size() - idx);

    json jsobj;
    jsobj["msgid"] = ONE_CHAT_MSG;
    jsobj["id"] = g_current_user.getId();
    jsobj["name"] = g_current_user.getName();
    jsobj["to"] = friendid;
    jsobj["msg"] = message;
    jsobj["time"] = getCurrentTime();

    string jsstr = jsobj.dump();
    send(fd, jsstr.c_str(), jsstr.length() + 1, 0);
}

void addFriend(int fd, string str) {
    int friend_id = stoi(str.c_str());

    json jsobj;
    jsobj["msgid"] = ADD_FRIEND_MSG;
    jsobj["id"] = g_current_user.getId();
    jsobj["friendid"] = friend_id;

    string jsstr = jsobj.dump();
    send(fd, jsstr.c_str(), jsstr.length() + 1, 0);
}

void createGroup(int fd, string str) {
    int idx = str.find(":");
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json jsobj;
    jsobj["msgid"] = CREATE_GROUP_MSG;
    jsobj["id"] = g_current_user.getId();
    jsobj["groupname"] = groupname;
    jsobj["groupdesc"] = groupdesc;

    string jsstr = jsobj.dump();
    send(fd, jsstr.c_str(), jsstr.length() + 1, 0);
}

void addGroup(int fd, string str) {
    int groupid = stoi(str.c_str());

    json jsobj;
    jsobj["msgid"] = ADD_GROUP_MSG;
    jsobj["id"] = g_current_user.getId();
    jsobj["groupid"] = groupid;

    string jsstr = jsobj.dump();
    send(fd, jsstr.c_str(), jsstr.length() + 1, 0);
}

void groupChat(int fd, string str) {
    int idx = str.find(":");
    int groupid = stoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json jsobj;
    jsobj["msgid"] = GROUP_CHAT_MSG;
    jsobj["id"] = g_current_user.getId();
    jsobj["name"] = g_current_user.getName();
    jsobj["groupid"] = groupid;
    jsobj["msg"] = message;
    jsobj["time"] = getCurrentTime();

    string jsstr = jsobj.dump();
    send(fd, jsstr.c_str(), jsstr.length() + 1, 0);
}

void logout(int fd, string str) {
    json jsobj;
    jsobj["msgid"] = LOGOUT_MSG;
    jsobj["id"] = g_current_user.getId();

    string jsstr = jsobj.dump();
    send(fd, jsstr.c_str(), jsstr.length() + 1, 0);
}
