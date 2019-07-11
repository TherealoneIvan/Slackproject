#ifndef MUTEX_H
#define MUTEX_H
#include <mutex>
using FastMutex = std::mutex;

class ScopedLock {
public:
    ScopedLock(std::mutex &m)
        : m(m)
    {
        m.lock();
    }
    ~ScopedLock() {
        m.unlock();
    }
private:
    std::mutex &m;
};
#endif
