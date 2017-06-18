//
// Created by ht on 17-6-17.
//

#ifndef LIBNET_TCPCLIENT_H
#define LIBNET_TCPCLIENT_H


#include "Mutex.h"
#include "TcpConnection.h"

#include <boost/noncopyable.hpp>


class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient :boost::noncopyable {
public:
    TcpClient(EventLoop* loop,const InetAddress& serverAddr, const std::string& nameArg);

    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        MutexLockGuard lock(mutex_);
        return connection_;
    }

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallbackCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }


private:
    void newConnection(int sockfd);

    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop *loop_;
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallbackCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;
    bool connect_;
    int nextConnId_;
    mutable MutexLock mutex_;
    TcpConnectionPtr connection_;



};


#endif //LIBNET_TCPCLIENT_H
