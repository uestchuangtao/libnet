//
// Created by ht on 17-6-11.
//

#ifndef LIBNET_CONNECTOR_H
#define LIBNET_CONNECTOR_H


#include "InetAddress.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class EventLoop;
class Channel;


class Connector : boost::noncopyable, public boost::enable_shared_from_this {
public:
    typedef std::function<void(int sockfd)> NewConnectionCallback;

    Connector(EventLoop* loop,const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    const InetAddress& serverAddress() const
    {
        return serverAddr_;
    }

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

private:
    enum States {kDisconnected, kConnecting, kConnected};
    static const int kMaxRetryDelayMs;
    static const int kInitRetryDelayMs;

    void setState(States state)
    {
        state_ = state;
    }

    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;
    States state_;
    boost::scoped_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;


};


#endif //LIBNET_CONNECTOR_H
