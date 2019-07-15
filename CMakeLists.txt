cmake_minimum_required(VERSION 2.7)

include_directories(slacklib)

add_subdirectory(slacklib)

add_executable(prog main.cpp)

target_link_libraries(prog PUBLIC slacklibrary)

