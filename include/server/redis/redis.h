#ifndef INCLUDE_REDIS_H_
#define INCLUDE_REDIS_H_

#include <hiredis/hiredis.h>
#include <functional>
#include <string>

using namespace std;

class Redis {
 public:
    Redis();
    ~Redis();

    bool connect();
    bool publish(int channel, string message);
    bool subscribe(int channel);
    bool unsubscribe(int channel);
    void observerChannelMsg();
    void setNotifyHandler(function<void(int, string)> fn);

 private:
    redisContext* _publish_txt;
    redisContext* _subscribe_txt;
    function<void(int, string)> _notify_msg_handler;
};

#endif  // INCLUDE_REDIS_H_
