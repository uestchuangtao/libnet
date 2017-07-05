//
// Created by ht on 17-6-15.
//

#include "TcpServer.h"

#include "SocketsOps.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include <stdio.h> // snprintf

#include <boost/bind.hpp>

#include <assert.h>

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
    :loop_(loop),
     ipPort_(listenAddr.toIpPort()),
     name_(nameArg),
     acceptor_(new Acceptor(loop_,listenAddr,option == kReusePort)),
     threadPool_(new EventLoopThreadPool(loop_, name_)),
     connectionCallback_(defaultConnectionCallback),
     messageCallback_(defaultMessageCallback),
     nextConnId_(1)

{
    acceptor_->setNewConnectionCallback(boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();

    for(auto iter:connections_)
    {
        TcpConnectionPtr conn = iter.second;
        iter.second.reset();
        conn->getLoop()->runInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
        conn.reset();
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if(started_.getAndSet(1) == 0)
    {
        threadPool_->start(threadInitCallback_);

        assert(!acceptor_->listening());
        loop_->runInLoop(boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}


void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf,sizeof(buf),"-%s#%d", ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(boost::bind(&TcpServer::removeConnection, this, _1));

    ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished, conn));

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
}
