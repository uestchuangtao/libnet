//
// Created by ht on 17-6-18.
//

#include "daytime.h"
#include "Timestamp.h"

#include <boost/bind.hpp>

#include <string>
#include <iostream>


DaytimeServer::DaytimeServer(EventLoop *loop, const InetAddress &listenAddr)
        : server_(loop, listenAddr, "DayTimeServer")
{
    server_.setConnectionCallback(boost::bind(&DaytimeServer::onConnection, this, _1));
    server_.setMessageCallback(boost::bind(&DaytimeServer::onMessage, this, _1, _2, _3));
}

void DaytimeServer::start()
{
    server_.start();
}

void DaytimeServer::onConnection(const TcpConnectionPtr &conn)
{
    conn->send(Timestamp::now().toFormattedString() + "\n");
    std::cout << "DayTimeServer send" << std::endl;
    conn->shutdown();
}

void DaytimeServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg(buf->retrieveAllAsString());
    std::cout << conn->name() << " discards " << msg << " " << time.toString() << std::endl;
}
