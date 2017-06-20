//
// Created by ht on 17-6-18.
//

#ifndef LIBNET_DAYTIME_H
#define LIBNET_DAYTIME_H


#include "TcpServer.h"

class DaytimeServer {
public:
    DaytimeServer(EventLoop *loop, const InetAddress &listenAddr);

    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

    TcpServer server_;

};


#endif //LIBNET_DAYTIME_H
