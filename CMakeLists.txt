cmake_minimum_required(VERSION 3.7)
project(libnet)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        Condition.cpp
        Condition.h
        Mutex.h
        BlockingQueue.h Exception.cpp Exception.h BoundedBlockingQueue.h Thread.cpp Thread.h ThreadPool.cpp ThreadPool.h InetAddress.cpp InetAddress.h SocketsOps.cpp SocketsOps.h Socket.cpp Socket.h Acceptor.cpp Acceptor.h)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.54.0 COMPONENTS log thread system regex REQUIRED)

add_executable(libnet ${SOURCE_FILES})
target_link_libraries(libnet ${Boost_LIBRARIES})