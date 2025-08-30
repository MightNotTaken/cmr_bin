#include <expire.h>

Expire::Expire() {
    this->_expired = false;
}

void Expire::expire() {
    this->_expired = true;
    invoke(this->expireCallback);
}

bool Expire::isExpired() {
    return this->_expired;
}

void Expire::onExpire(std::function<void()> callback) {
    this->expireCallback = callback;
}