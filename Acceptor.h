//
// Created by ht on 17-6-8.
//

#ifndef LIBNET_ACCEPTOR_H
#define LIBNET_ACCEPTOR_H

#include "Socket.h"
#include "Channel.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

class EventLoop;
class InetAddress;

class Acceptor :boost::noncopyable {
public:

    typedef boost::function<void(int sockfd,
                                 const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr,bool reuseport);

    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listening() const
    {
        return listenning;
    }

    void listen();


private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectioncallback_;
    bool listenning;

};


#endif //LIBNET_ACCEPTOR_H
