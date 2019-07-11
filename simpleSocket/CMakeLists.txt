cmake_minimum_required(VERSION 2.7)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

set(poco_libs
        ${CMAKE_SOURCE_DIR}/extern/poco/lib/libPocoJSON.a
        ${CMAKE_SOURCE_DIR}/extern/poco/lib/libPocoNet.a
        ${CMAKE_SOURCE_DIR}/extern/poco/lib/libPocoUtil.a
        ${CMAKE_SOURCE_DIR}/extern/poco/lib/libPocoFoundation.a
        )

add_executable(simple_socket simple.cpp)

target_link_libraries(simple_socket ${poco_libs} pthread)

target_include_directories(simple_socket PRIVATE SYSTEM
${CMAKE_SOURCE_DIR}/extern/poco/Foundation/include/
${CMAKE_SOURCE_DIR}/extern/poco/Net/include/
${CMAKE_SOURCE_DIR}/extern/poco/Util/include/
${CMAKE_SOURCE_DIR}/extern/poco/JSON/include/)
