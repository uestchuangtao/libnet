//
// Created by ht on 17-6-8.
//

#ifndef LIBNET_SOCKET_H
#define LIBNET_SOCKET_H

#include <boost/noncopyable.hpp>

class InetAddress;

class Socket :boost::noncopyable {
public:
    explicit Socket(int sockfd)
            :sockfd_(sockfd)
    {

    }

    int fd() const
    {
        return sockfd_;
    }

    void bindAddress(const InetAddress& localAddr);

    //void connect(const InetAddress& peerAddr);

    void listen();

    int accept(InetAddress* peerAddr);

    void shutdownWrite();

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);

    void setTcpNoDelay(bool on);
private:
    const int sockfd_;

};


#endif //LIBNET_SOCKET_H
