//
// Created by ht on 17-6-18.
//

#include <iostream>

#include "daytime.h"
#include "EventLoop.h"
#include "InetAddress.h"


int main(int argc, char *argv[])
{
    std::cout << "hello DaytimeServer" << std::endl;

    EventLoop loop;

#if 1
    InetAddress listenAddr(2016);
    DaytimeServer server(&loop, listenAddr);
    server.start();
#endif
    loop.loop();
    return 0;
}

