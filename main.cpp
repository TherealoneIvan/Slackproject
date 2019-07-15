#include "slack.h"
#include <iostream>

int main() {
    SlackInterceptor si;
    si.setSocket("http://localhost:9222/json");
    si.listenAndCatch();
}