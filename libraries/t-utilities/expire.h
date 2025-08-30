#ifndef EXPIRE_H__
#define EXPIRE_H__
#include <functional>
#include <definitions.h>
class Expire {
    bool _expired;
    std::function<void()> expireCallback;
public:
    Expire();;
    void expire();
    bool isExpired();
    void onExpire(std::function<void()>);
};

#endif