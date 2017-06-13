cmake_minimum_required(VERSION 3.7)
project(libnet)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        Condition.cpp
        Condition.h
        Mutex.h
        BlockingQueue.h
        Exception.cpp
        Exception.h
        BoundedBlockingQueue.h
        Thread.cpp
        Thread.h
        ThreadPool.cpp
        ThreadPool.h
        InetAddress.cpp
        InetAddress.h
        SocketsOps.cpp
        SocketsOps.h
        Socket.cpp
        Socket.h
        Acceptor.cpp
        Acceptor.h
        Channel.cpp
        Channel.h
        EventLoop.cpp
        EventLoop.h
        Poller.cpp
        Poller.h
        PollPoller.cpp
        PollPoller.h
        EPollPoller.cpp
        EPollPoller.h
        Connector.cpp
        Connector.h)


set (BOOST_ROOT "/usr/include/boost")

#add include path
include_directories(/usr/include)

#add lib path
include_directories(/usr/lib)

add_executable(libnet ${SOURCE_FILES})

#lookup libboost_thread.a
target_link_libraries(libnet boost_thread boost_system)