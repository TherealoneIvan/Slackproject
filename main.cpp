#include "slack.h"
#include <iostream>

int main(int argc, char ** argv) {
    SlackInterceptor si;
    si.setAddress("https://localhost:9222/json");
    si.createListeners();
}