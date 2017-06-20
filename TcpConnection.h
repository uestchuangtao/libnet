//
// Created by ht on 17-6-14.
//

#ifndef LIBNET_TCPCONNECTION_H
#define LIBNET_TCPCONNECTION_H

#include "InetAddress.h"
#include "Buffer.h"
#include "Callbacks.h"

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/any.hpp>

#include <string>

class EventLoop;
class Socket;
class Channel;

class TcpConnection : boost::noncopyable, public boost::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, const std::string &name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);

    ~TcpConnection();

    EventLoop* getLoop() const
    {
        return loop_;
    }

    const std::string& name() const
    {
        return name_;
    }

    const InetAddress& localAddress() const
    {
        return localAddr_;
    }

    const InetAddress& peerAddress() const
    {
        return peerAddr_;
    }

    bool connected() const
    {
        return state_ == kConnected;
    }

    bool disconneted() const
    {
        return state_ == kDisconnected;
    }

    void setContext(const boost::any& context)
    {
        context_ = context;
    }

    const boost::any& getContext() const
    {
        return context_;
    }

    boost::any* getMutableConText()
    {
        return &context_;
    }

    void send(const std::string& message);

    void send(const char* buf, int len);

    void send(Buffer* buf);

    void shutdown();

    void forceClose();

    //void forceCloseWithDelay(double seconds);

    void setTcpNoDelay(bool on);

    void startRead();

    void stopRead();

    bool isReading() const
    {
        return reading_;
    }

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;

    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }

    Buffer* inputBuffer()
    {
        return &inputBuffer_;
    }

    Buffer* outputBuffer()
    {
        return &outputBuffer_;
    }

    void connectEstablished();

    void connectDestroyed();

private:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();

    void forceCloseInLoop();
    void setState(StateE s)
    {
        state_ = s;
    }
    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();

    EventLoop* loop_;
    std::string name_;
    StateE state_;
    boost::scoped_ptr<Socket> socket_;
    boost::scoped_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    boost::any context_;
    bool reading_;


};

//typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;


#endif //LIBNET_TCPCONNECTION_H
