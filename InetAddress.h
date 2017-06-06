//
// Created by ht on 17-6-5.
//

#ifndef LIBNET_INETADDRESS_H
#define LIBNET_INETADDRESS_H


#include <string>
#include <stdint.h>
#include <netinet/in.h>

class InetAddress {
public:
    InetAddress(const std::string &ip, uint16_t port);
    explicit  InetAddress(uint16_t port = 0, bool loopbackOnly = false);
    explicit InetAddress(const struct sockaddr_in& addr)
        :addr_(addr)
    {

    }

    std::string toIp() const;
    uint16_t toPort() const;
    std::string toIpPort() const;
    sa_family_t  family() const
    {
        return addr_.sin_family;
    }

    static bool resolve(const std::string& hostname, InetAddress *result);


private:
    struct sockaddr_in addr_;
};


#endif //LIBNET_INETADDRESS_H
