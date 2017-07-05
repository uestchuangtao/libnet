//
// Created by ht on 17-6-20.
//

#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpServer.h"
#include "Timestamp.h"
#include "Atomic.h"
#include "TimerId.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <stdio.h>
#include <iostream>

using std::cout;
using std::endl;

int numThreads = 5;

class DiscardServer : boost::noncopyable {
public:
    DiscardServer(EventLoop *loop, const InetAddress &listenAddr)
            : loop_(loop),
              server_(loop_, listenAddr, "DiscardServer"),
              oldCounter_(0),
              startTime_(Timestamp::now())
    {
        server_.setConnectionCallback(boost::bind(&DiscardServer::onConnection, this, _1));
        server_.setMessageCallback(boost::bind(&DiscardServer::onMessage, this, _1, _2, _3));

        server_.setThreadNum(numThreads);

        /*timerId_ = */loop_->runEvery(10.0, boost::bind(&DiscardServer::printThroughput, this));
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        cout << conn->name() << " " << conn->localAddress().toIpPort() << "->" << conn->peerAddress().toIpPort()
             << endl;
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        size_t len = buf->readableBytes();
        transferred_.add(len);
        receivedMessages_.incrementAndGet();
        buf->retrieveAll();
    }

    void printThroughput()
    {
        Timestamp endTime = Timestamp::now();
        int64_t newCounter = transferred_.get();
        int64_t bytes = newCounter - oldCounter_;
        int64_t msgs = receivedMessages_.getAndSet(0);
        double time = timeDifference(endTime, startTime_);

        printf("%4.3f Mb/s %4.3f Ki Msgs/s %6.2f bytes per msg\n",
               static_cast<double>(bytes) / time / 1024 / 1024,
               static_cast<double>(msgs) / time / 1024,
               static_cast<double>(bytes) / static_cast<double>(msgs));

        startTime_ = endTime;
        oldCounter_ = newCounter;

        //loop_->Cancel(timerId_);

    }

    TcpServer server_;
    EventLoop *loop_;
    AtomicInt64 transferred_;
    AtomicInt64 receivedMessages_;
    int64_t oldCounter_;
    Timestamp startTime_;
    //TimerId timerId_;
};

int main(int argc, char **argv)
{
    EventLoop loop;
    InetAddress listenAddr(2009);
    DiscardServer server(&loop, listenAddr);

    server.start();

    loop.loop();
}