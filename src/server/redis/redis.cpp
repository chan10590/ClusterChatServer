#include "redis.h"
#include <iostream>
#include <thread>

Redis::Redis() : _publish_txt(nullptr), _subscribe_txt(nullptr) {}
Redis::~Redis() {
    if (_publish_txt) {
        redisFree(_publish_txt);
    }
    if (_subscribe_txt) {
        redisFree(_subscribe_txt);
    }
}

bool Redis::connect() {
    _publish_txt = redisConnect("127.0.0.1", 6379);
    if (!_publish_txt) {
        cerr << "connect redis fail!" << endl;
        return false;
    }
    _subscribe_txt = redisConnect("127.0.0.1", 6379);
    if (!_subscribe_txt) {
        cerr << "connect redis fail!" << endl;
        return false;
    }

    thread thd([&]() { observerChannelMsg(); });
    thd.detach();
    cout << "connect redis success!" << endl;
    return true;
}
bool Redis::publish(int channel, string message) {
    redisReply* reply = static_cast<redisReply*>(redisCommand(_publish_txt, "PUBLISH %d %s", channel, message.c_str()));
    if (!reply) {
        cerr << "publish command fail!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}
bool Redis::subscribe(int channel) {
    if (redisAppendCommand(_subscribe_txt, "SUBSCRIBE %d", channel) == REDIS_ERR) {
        cerr << "subscribe command fail!" << endl;
        return false;
    }
    int done = 0;
    while (!done) {
        if (redisBufferWrite(_subscribe_txt, &done) == REDIS_ERR) {
            cerr << "subscribe command fail!" << endl;
            return false;
        }
    }
    return true;
}
bool Redis::unsubscribe(int channel) {
    if (redisAppendCommand(_subscribe_txt, "UNSUBSCRIBE %d", channel) == REDIS_ERR) {
        cerr << "unsubscribe command fail!" << endl;
        return false;
    }
    int done = 0;
    while (!done) {
        if (redisBufferWrite(_subscribe_txt, &done) == REDIS_ERR) {
            cerr << "unsubscribe command fail!" << endl;
            return false;
        }
    }
    return true;
}
void Redis::observerChannelMsg() {
    redisReply* reply = nullptr;
    while (redisGetReply(_subscribe_txt, reinterpret_cast<void**>(&reply)) == REDIS_OK) {
        if (reply && reply->element) {
            if (reply->element[2] && reply->element[2]->str) {
                cout << reply->element[2]->str << endl;
                _notify_msg_handler(stoi(reply->element[1]->str), reply->element[2]->str);
            }
        } else {
            cout << "redisGetReply no response" << endl;
        }
        freeReplyObject(reply);
    }
    cerr << "=================observerChannelMsg quit==============" << endl;
}
void Redis::setNotifyHandler(function<void(int, string)> fn) { _notify_msg_handler = fn; }
