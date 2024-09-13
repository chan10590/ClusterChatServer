#include "chatserver.h"
#include <muduo/base/Logging.h>
#include <functional>
#include <json.hpp>
#include <string>
#include "chatservice.h"

using namespace std;
using namespace muduo;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop) {
    // 注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, placeholders::_1));
    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, placeholders::_1,
                                         placeholders::_2, placeholders::_3));
    // 设置线程数
    _server.setThreadNum(4);
}

void ChatServer::start() { _server.start(); }

void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    if (!conn->connected()) {
        ChatService::instance()->clientExceptClose(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) {
    string buf = buffer->retrieveAllAsString();
    // LOG_INFO << buf;
    // 数据反序列化
    json js = json::parse(buf);
    // 根据msgid获取业务
    MsgHandler handler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    handler(conn, js, time);
}
