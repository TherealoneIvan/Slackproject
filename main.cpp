#include "slack.h"
#include <iostream>
#include <unistd.h>

int main(int argc, char ** argv) {
    pid_t pid;
    SlackInterceptor si;
    si.setAddress("https://localhost:9222/json");
    si.start();
    /*
    if ((pid = fork()) < 0) {
        std::cout << "error" <<"\n";
    } else if (pid == 0) {
        //SlackInterceptor si;
        //si.setAddress("https://localhost:9222/json");
        si.createListeners();
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        si.stop();
        std::cout << "abc" << "\n";
        si.start();
    }
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    //si.stop();
    //si.start();
}