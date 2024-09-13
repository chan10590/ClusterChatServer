#ifndef INCLUDE_CHATSERVER_H_
#define INCLUDE_CHATSERVER_H_

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer {
 public:
    // 初始化聊天服务器
    ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
    ~ChatServer() = default;
    // 启动服务器
    void start();

 private:
    // 连接的回调
    void onConnection(const TcpConnectionPtr& conn);
    // 读写事件相关消息的回调
    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
    TcpServer _server;
    EventLoop* _loop;
};
#endif  // INCLUDE_CHATSERVER_H_
