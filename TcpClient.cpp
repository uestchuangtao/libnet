//
// Created by ht on 17-6-17.
//

#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <stdio.h>

namespace detail
{
    void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
    {
        loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed,conn));
    }

    void removeConnector(const Connector& connector)
    {
        //TODO
    }
}

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr, const std::string &nameArg)
    :loop_(loop),
     connector_(new Connector(loop_,serverAddr)),
     name_(nameArg),
     connectionCallback_(defaultConnectionCallback),
     messageCallbackCallback_(defaultMessageCallback),
     retry_(false),
     connect_(true),
     nextConnId_(1)
{
    connector_->setNewConnectionCallback(boost::bind(&TcpClient::newConnection,this,_1));

}

TcpClient::~TcpClient()
{
    TcpConnectionPtr conn;
    bool unique = false;
    {
        MutexLockGuard lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if(conn)
    {
       assert(loop_ == conn->getLoop());
        CloseCallback cb = boost::bind(&detail::removeConnection,loop_,_1);
        loop_->runInLoop(boost::bind(&TcpConnection::setCloseCallback,conn,cb));
        if(unique)
        {
            conn->forceClose();
        }
    }
    else
    {
        connector_->stop();
        loop_->runAfter(1,boost::bind(&detail::removeConnector,connector_));
    }
}

void TcpClient::connect()
{
    connect_  =true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;
    {
        MutexLockGuard lock(mutex_);
        if(connection_)
        {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf,sizeof(buf),":%s#%d", peerAddr.toIpPort().c_str(),nextConnId_);
    nextConnId_++;
    std::string connName = name_ + buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_,connName,sockfd,localAddr,peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallbackCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(boost::bind(&TcpClient::removeConnection, this, _1));
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(conn == connection_);
        connection_.reset();
    }

    loop_->queueInLoop(boost::bind(&TcpConnection::connectDestroyed,conn));

    if(retry_ && connect_)
    {
        connector_->restart();
    }
}

