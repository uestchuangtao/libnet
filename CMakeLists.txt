cmake_minimum_required(VERSION 3.7)
project(libnet)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        Condition.cpp
        Condition.h
        Mutex.h
        BlockingQueue.h Exception.cpp Exception.h)

add_executable(libnet ${SOURCE_FILES})