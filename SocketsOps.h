//
// Created by ht on 17-6-5.
//

#ifndef LIBNET_SOCKETSOPS_H
#define LIBNET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace sockets{
    int createNonBlockingOrDie();

    void close(int sockfd);

    int connect(int sockfd, const struct sockaddr* addr);

    void bindOrDie(int sockfd, const struct sockaddr* addr);

    void listenOrDie(int sockfd);

    int accecpt(int sockfd,struct sockaddr* addr);

    void shutdownWrite(int sockfd);

    void toIpPort(char *buf, size_t size, const struct sockaddr* addr);

    void toIp(char *buf, size_t size,const struct sockaddr* addr);

    void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

    int getSocketError(int sockfd);

    struct sockaddr_in getLocalAddr(int sockfd);

    struct sockaddr_in getPeerAddr(int sockfd);

    bool isSelfConnnect(int sockfd);

    size_t read(int sockfd,void *buf, size_t len);
    size_t readv(int sockfd,const struct iovec *iov, int iovcnt);
    size_t write(int sockfd, void* buf, size_t len);

}


#endif //LIBNET_SOCKETSOPS_H
