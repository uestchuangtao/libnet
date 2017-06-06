//
// Created by ht on 17-6-5.
//

#include "SocketsOps.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stdio.h> //snprintf

namespace {
    typedef struct sockaddr SA;

    void setNonBlockAndCloseOnExec(int sockfd)
    {
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;

        int ret = ::fcntl(sockfd, F_SETFL, flags);

        if(!ret)
        {
            //TODO: LOG_SYS<<"SetNonBlockAndCloseOnExec::fcntl";
        }

        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= O_CLOEXEC;

        ret = ::fcntl(sockfd, F_SETFD, flags);

        if(!ret)
        {
            //TODO: LOG_SYS<<"SetNonBlockAndCloseOnExec::fcntl";
        }
    }
}

int sockets::createNonBlockingOrDie()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        //TODO: LOG_SYS<<"createNonBlockingOrDie::socket";
    }

    setNonBlockAndCloseOnExec(sockfd);

    return sockfd;

}

void sockets::close(int sockfd)
{
    if(::close(sockfd) < 0)
    {
        //TODO: LOG_SYS<<"sockets::close";
    }
}

int sockets::connect(int sockfd, const struct sockaddr *addr)
{
    return ::connect(sockfd,addr,static_cast<socklen_t>(sizeof(struct sockaddr)));
}

void sockets::bindOrDie(int sockfd, const struct sockaddr *addr)
{
    if(::bind(sockfd,addr) < 0)
    {
        //TODO: LOG_SYS<<"sockets::bind";
    }
}

void sockets::listenOrDie(int sockfd)
{
    if(::listen(sockfd,5) < 0)
    {
        //TODO: LOG_SYS<<"sockets::listen";
    }
}

int sockets::accecpt(int sockfd, struct sockaddr *addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = accept(sockfd,addr,&addrlen);
    setNonBlockAndCloseOnExec(connfd);
    if(connfd < 0){
        //TODO: LOG_SYS<<"sockets::accept";
    }
    return connfd;
}

void sockets::shutdownWrite(int sockfd)
{
    if(::shutdown(sockfd,SHUT_WR) < 0)
    {
        //TODO: LOG_SYS<<"sockets::shutdown";
    }
}

void sockets::toIpPort(char *buf, size_t size, const struct sockaddr* addr)
{
    toIp(buf,size,addr);
    struct sockaddr_in addr4 = static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
    size_t  end = ::strlen(buf);
    uint16_t  port = ntohs(addr4.sin_port);
    assert(size > end);
    snprintf(buf+end,size-end,":%u",port);
}

void sockets::toIp(char *buf, size_t size, const struct sockaddr* addr)
{
    struct sockaddr_in addr4 = static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
    if(::inet_ntop(AF_INET,&addr4.sin_addr,buf,static_cast<socklen_t>(size)) == NULL)
    {
        //TODO: LOG_SYS<<"sockets::inet_ntop";
    }
}

void sockets::fromIpPort(const char *ip, uint16_t port, struct sockaddr_in addr)
{
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(inet_pton(AF_IENT,&addr.sin_addr,ip) <= 0)
    {
        //TODO: LOG_SYS<<"sockets::inet_pton";
    }
}

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&optval,&optlen))
    {
        //TODO: LOG_SYS<<"sockets::getsockopt";
        return errno;
    }
    return optval;
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in localAddr;
    bzero(&localAddr,sizeof(localAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
    if(getsockname(sockfd,(SA*)&localAddr,&addrlen) < 0)
    {
        //TODO: LOG_SYS<<"sockets::getsockname";
    }
    return localAddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
    struct sockaddr_in peerAddr;
    bzero(&peer,sizeof(peerAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peerAddr));
    if(::getpeername(sockfd,(SA*)&peerAddr,&addrlen) < 0)
    {
        //TODO: LOG_SYS<<"sockets::getsockname";
    }
    return peerAddr;
}

bool sockets::isSelfConnnect(int sockfd)
{
    struct sockaddr_in localAddr = getLocalAddr(sockfd);
    struct sockaddr_in peerAddr = getPeerAddr(sockfd);
    return localAddr.sin_port == peerAddr.sin_port
           && localAddr.sin_addr.s_addr == peerAddr.sin_addr.s_addr;
}

size_t sockets::read(int sockfd, void *buf, size_t len)
{
    return ::read(sockfd,buf,len);
}

size_t sockets::readv(int sockfd,const struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd,iov,iovcnt);
}

size_t sockets::write(int sockfd, void *buf, size_t len)
{
    return ::write(sockfd,buf,len);
}






