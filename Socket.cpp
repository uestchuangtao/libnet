//
// Created by ht on 17-6-8.
//

#include "Socket.h"
#include "SocketsOps.h"
#include "InetAddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>  //for TCP_NODELAY

void Socket::listen()
{
    sockets::listenOrDie(sockfd_);
}

void Socket::bindAddress(const InetAddress &localAddr)
{
    sockets::bindOrDie(sockfd_,localAddr.getSockAddr());
}

int Socket::accept(InetAddress *peerAddr)
{
    struct sockaddr_in addr;
    bzero(&addr,sizeof(struct sockaddr));
    int connfd = sockets::accecpt(sockfd_,&addr);
    if( connfd >=0)
    {
        peerAddr->setSockAddr(addr);
    }
    return connfd;

}

void Socket::shutdownWrite()
{
    sockets::shutdownWrite(sockfd_);
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) != 0)
    {
        //TODO: LOG_SYS<<"Socket::setKeepAlive";
    }
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) != 0)
    {
        //TODO: LOG_SYS<<"Socket::setReuseAddr";
    }
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, optlen) != 0)
    {
        //TODO: LOG_SYS<<"Socket::setResuePort";
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, optlen) != 0)
    {
        //TODO: LOG_SYS<<"Socket::setTcpNoDelay";
    }
}



