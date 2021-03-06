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
        Connector.h
        CurrentThread.h
        Timestamp.h
        Timestamp.cpp
        TcpConnection.cpp
        TcpConnection.h
        Buffer.cpp
        Buffer.h
        Callbacks.h
        TcpServer.cpp
        TcpServer.h
        Atomic.h
        TcpClient.cpp
        TcpClient.h
        EventLoopThread.cpp
        EventLoopThread.h
        EventLoopThreadPool.cpp
        EventLoopThreadPool.h
        TimerQueue.cpp
        TimerQueue.h
        TimerId.h
        Timer.cpp
        Timer.h)

set(DAYTIME_SOURCE_FILES
        DaytimeServer.cpp daytime.cpp daytime.h)

set(DISCARD_SERVER_SOURCE_FILES
        DiscardServer.cpp)

set(DISCARD_CLIENT_SOURCE_FILES
        DiscardClient.cpp)

set(USB_SERVER_SOURCE_FILES
        UsbServer.cpp)


set (BOOST_ROOT "/usr/include/boost")

#add include path
include_directories(/usr/include)

#add lib path
include_directories(/usr/lib)

add_executable(libnet ${SOURCE_FILES})
#lookup libboost_thread.a
target_link_libraries(libnet pthread)

add_executable(daytime_server ${SOURCE_FILES} ${DAYTIME_SOURCE_FILES})
target_link_libraries(daytime_server pthread)

add_executable(discard_server ${SOURCE_FILES} ${DISCARD_SERVER_SOURCE_FILES})
target_link_libraries(discard_server pthread)

add_executable(discard_client ${SOURCE_FILES} ${DISCARD_CLIENT_SOURCE_FILES})
target_link_libraries(discard_client pthread)

add_executable(usb_server ${SOURCE_FILES} ${USB_SERVER_SOURCE_FILES})
target_link_libraries(usb_server pthread)