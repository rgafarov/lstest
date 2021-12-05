cmake_minimum_required(VERSION 3.16)

project(lstest)

add_executable(lstest main.cpp)

target_link_libraries(lstest pthread)