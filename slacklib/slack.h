#include <string>

#ifndef NSOKET_SIMPLE_H
#define NSOKET_SIMPLE_H

class SlackInterceptor {
 private:
    std::string socket;
 public:
    void listenAndCatch();
    void setSocket(std::string str);
};
#endif //NSOKET_SIMPLE_H
