cmake_minimum_required(VERSION 2.7)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

set(poco_libs
        ${CMAKE_SOURCE_DIR}/slacklib/extern/poco/lib/libPocoJSON.a
        ${CMAKE_SOURCE_DIR}/slacklib//extern/poco/lib/libPocoNet.a
        ${CMAKE_SOURCE_DIR}/slacklib/extern/poco/lib/libPocoUtil.a
        ${CMAKE_SOURCE_DIR}/slacklib/extern/poco/lib/libPocoFoundation.a
        )

include_directories(${CMAKE_SOURCE_DIR}/slacklib/circularBuffer)

add_subdirectory(${CMAKE_SOURCE_DIR}/slacklib/circularBuffer)

add_library(slacklibrary slack.cpp slack.h)

target_link_libraries(slacklibrary ${poco_libs} pthread)

target_link_libraries(slacklibrary PUBLIC circularBuffer)

target_include_directories(slacklibrary PUBLIC
${CMAKE_SOURCE_DIR}/slacklib/extern/poco/Foundation/include/
${CMAKE_SOURCE_DIR}/slacklib//extern/poco/Net/include/
${CMAKE_SOURCE_DIR}/slacklib/extern/poco/Util/include/
${CMAKE_SOURCE_DIR}/slacklib/extern/poco/JSON/include/)

