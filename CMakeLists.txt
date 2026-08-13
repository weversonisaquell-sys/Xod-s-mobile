cmake_minimum_required(VERSION 3.22.1)
project(xodocore)

add_library(xodocore SHARED
    native-lib.cpp
    cpu8086.cpp
)

find_library(log-lib log)

target_link_libraries(xodocore ${log-lib})
