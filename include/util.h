#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

enum EnMsgType {
    LOGIN_MSG = 1,   // msgid=1, 登录
    REG_MSG,         // msgid=2, 注册
    REG_MSG_ACK,     // msgid=3, 登录确认
    LOGIN_MSG_ACK,   // msgid=4, 注册确认
    ONE_CHAT_MSG,    // msgid=5, 一对一聊天
    ADD_FRIEND_MSG,  // msgid=6, 添加好友

    CREATE_GROUP_MSG,  // msgid=7, 创建群
    ADD_GROUP_MSG,     // msgid=8, 加入群
    GROUP_CHAT_MSG,    // msgid=9, 群聊天
    LOGOUT_MSG,        // msgid=10, 注销
};

#endif  // INCLUDE_UTIL_H_
