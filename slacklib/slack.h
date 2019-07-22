#ifndef NSOKET_SIMPLE_H
#define NSOKET_SIMPLE_H

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/URI.h"
#include "circularBuffer/circularBuffer.h"

#include <string>
#include <thread>
#include <vector>


typedef struct tmessage {
    std::string workplaceName;
    std::string user;
    std::string to;
    std::string text;
    std::string time;
    bool isSender;
} message;

class SlackInterceptor {
 public:
    SlackInterceptor() {
        socket  = "https:/localhost:0001/json";
    }
    void createListeners();
    void listenAndCatch(Poco::JSON::Object::Ptr object, Poco::URI uri, int index);
    void printInfo();
    void setSocket(std::string str);
    void fillInfo(std::string & workplace, std::string & name, std::string & addresse, std::string & body,
                    std::string & timestamp, bool wasSentBySessionOwner);
    void storageService();
 private:
    std::string socket;
    std::vector<std::thread> listeners;
    circular_buffer<message> storage;
    Poco::JSON::Parser parser;
};
#endif //NSOKET_SIMPLE_H
