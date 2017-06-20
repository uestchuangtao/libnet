//
// Created by ht on 17-6-14.
//

#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>


#include <errno.h>
#include <assert.h>

#include <iostream>  // for test

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    //TODO:
    /*LOG_TRACE<<conn->localAddress().toIpPort()<<" -> "
             << conn->peerAddress().toIpPort() << " is "<<(conn->connected() ? "UP":"DOWN");*/
}

void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    :loop_(loop),
     name_(name),
     state_(kConnecting),
     socket_(new Socket(sockfd)),
     channel_(new Channel(loop,sockfd)),
     localAddr_(localAddr),
     peerAddr_(peerAddr),
     highWaterMark_(64*1024*1024),
     reading_(true)

{
    channel_->setReadCallback(boost::bind(&TcpConnection::handleRead,this,_1));
    channel_->setWriteCallback(boost::bind(&TcpConnection::handleWrite,this));
    channel_->setErrorCallback(boost::bind(&TcpConnection::handleError,this));
    channel_->setCloseCallback(boost::bind(&TcpConnection::handleClose,this));

    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    assert(state_ == kDisconnected);
}

void TcpConnection::send(const char *buf, int len)
{
    send(std::string(buf,len));
}

void TcpConnection::send(const std::string &message)
{
    if(state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::send(Buffer *buf)
{
    if(state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message)
{
    sendInLoop(message.c_str(),message.size());
}

void TcpConnection::sendInLoop(const void *data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(state_ == kDisconnected)
    {
        //TODO: LOG_WARN<<"disconnected, give up writing";
        return;
    }
    //TODO: if nothing in output queue, try writing directly
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = sockets::write(channel_->fd(),data,len);
        if(nwrote >= 0)
        {
            remaining -= nwrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                //TODO:: why? shared_from_this
                loop_->queueInLoop(boost::bind(writeCompleteCallback_,shared_from_this()));

            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }
    assert(remaining <= len);
    if(!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(boost::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data+nwrote), remaining);
        if(!channel_->isWriting())
            channel_->enableWriting();
    }
}

void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose()
{
    if(state_ == kDisconnecting || state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(boost::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if(state_ == kDisconnecting || state_ == kConnected)
    {
        handleClose();
    }
}

const char* TcpConnection::stateToString() const
{
    switch(state_)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnected:
            return " kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        case kConnecting:
            return "kConnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop(boost::bind(&TcpConnection::startReadInLoop,this));
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if(!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
    }
    reading_ = true;
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(boost::bind(&TcpConnection::stopReadInLoop,this));
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if(reading_ || channel_->isReading())
    {
        channel_->disableReading();
    }
    reading_ = false;
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);

    //TODO:  why???
    channel_->tie(shared_from_this());
    channel_->enableReading();
    //TODO:
    std::cout << "In connectEstablished::connectionCallback" << std::endl;
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        //TODO: why???
        std::cout << "In connectDestroyed::connectionCallback" << std::endl;
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&savedErrno);
    if(n > 0)
    {
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        //TODO:LOG_SYS << "TcpConnetion::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting())
    {
        ssize_t n = sockets::write(channel_->fd(), outputBuffer_.peek(),outputBuffer_.readableBytes());
        if(n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0)
            {
                // disableWriting
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
                if(state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
           //TODO: LOG_SYSERR<<"TCPConnection::handleWrite";
        }
    }
    else
    {
        //TODO: LOG_TRACE<<"Connection fd = "<< channel_->fd() <<"is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    assert(state_ == kDisconnecting || state_ == kDisconnected);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());

    //TODO: connectionCallback for what
    std::cout << "In handleClose::connectionCallback" << std::endl;
    connectionCallback_(guardThis);

    closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    // LOG_ERROR << "TcpConnection::handleError["<<name_<<"] - SO_ERROR = " << err << " " <<strerror_tl(err);
}
