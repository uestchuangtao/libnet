//
// Created by ht on 17-6-11.
//

#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <string.h>


const int Connector::kMaxRetryDelayMs = 30*1000;
const int Connector::kInitRetryDelayMs = 500;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{

}

Connector::~Connector()
{
    assert(!channel_);
}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(boost::bind(&Connector::startInLoop,this));
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if(connect_)
    {
        connect();
    }
    else
    {
        //TODO: LOG_SYS<<"do not connect"
    }

}


void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(boost::bind(&Connector::stopInLoop,this));
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnecting) //TODO: this is important
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = sockets::createNonBlockingOrDie();
    int ret = sockets::connect(sockfd,serverAddr_.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;
        default:
            //TODO: LOG_SYS<<"unexpected error in Connector::startInLoop " << savedErrNo;
            sockets::close(sockfd);
            break;
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_,sockfd));
    channel_->setWriteCallback(boost::bind(&Connector::handleWrite,this));
    channel_->setErrorCallback(boost::bind(&Connector::handleError,this));
    channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    //TODO: why runInLoop??? because channel is handling Event, resetChannel is to ~Channel, ~Channel assert(!eventHandling_)
    loop_->queueInLoop(boost::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::handleWrite()
{
    if(state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if(err)
        {
            //TODO: LOG_WARN<<"Connector::handleWrite - SO_ERROR = " << err << " " << strerror_tl(err);
            retry(sockfd);
        }
        else if(sockets::isSelfConnnect(sockfd))
        {
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if(connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                sockets::close(sockfd);
            }
        }
    }
    else
    {
        //what happened?
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError()
{
    //TODO: LOG_ERROR<<"Connector::handleError state= " << state_;
    if(state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        //TODO: LOG_TRACE<<"SO_ERROR= " << err << " "<<strerror_tl(error);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setState(kDisconnected);
    if(connect_)
    {
        //TODO: LOG_INFO<<"Connector::retry - Retry connecting to" << serverAddr_.toIpPort()
        // << "in"<<retryDelayMs_<< " milliseconds.";
        loop_->runAfter(retryDelayMs_/1000.0,boost::bind(&Connector::startInLoop,shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        //TODO: LOG_DEBUG<<"do not connect";
    }
}