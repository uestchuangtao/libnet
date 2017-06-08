//
// Created by ht on 17-6-8.
//

#include "Acceptor.h"
#include "SocketsOps.h"
#include "InetAddress.h"

#include <boost/bind.hpp>

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    :loop_(loop),
     acceptSocket_(sockets::createNonBlockingOrDie()),
     acceptChannel_(loop,acceptSocket_.fd()),
     listenning(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(boost::bind(&Acceptor::handleReadead,this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();

}

void Acceptor::listen()
{
    listenning = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0)
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd,peerAddr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
    else
    {
        //TODO: LOG_SYSERR<<"Acceptor::handleRead";
    }
}