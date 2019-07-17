#include "slack.h"
#include <iostream>

int main(int argc, char ** argv) {
    SlackInterceptor si;
    si.setSocket("https://localhost:9222/json");
    si.createListeners();
}