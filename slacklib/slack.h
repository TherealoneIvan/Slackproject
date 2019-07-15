#include <string>

#ifndef NSOKET_SIMPLE_H
#define NSOKET_SIMPLE_H

typedef struct tmessage {
    std::string user;
    std::string to;
    std::string text;
    std::string time;
    bool isSender;
} message;

class SlackInterceptor {
 private:
    std::string socket;
    message info;
 public:
    void listenAndCatch();
    void printInfo();
    void setSocket(std::string str);
    void fillInfo (std::string & name, std::string & addresse, std::string & body,
                    std::string & timestamp, bool wasSentBySessionOwner);
};
#endif //NSOKET_SIMPLE_H
