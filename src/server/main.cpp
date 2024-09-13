#include <signal.h>
#include <iostream>
#include "chatserver.h"
#include "chatservice.h"

using namespace std;

void resetHandler(int) {
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char* argv[]) {
    cout << "===available server cluster:" << endl;
    cout << "   127.0.0.1:9527" << endl;
    cout << "   127.0.0.1:9528" << endl;
    if (argc < 3) {
        cerr << "invalid command! format: ./Server 127.0.0.1 9527" << endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "a muduo-based ChatServer");

    server.start();
    loop.loop();

    return 0;
}
