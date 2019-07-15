#include "slack.h"
#include <iostream>

int main(int argc, char ** argv) {
    SlackInterceptor si;
    si.setSocket(argv[1]);
    si.listenAndCatch();
}