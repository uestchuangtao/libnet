//
// Created by ht on 17-6-5.
//

#include "InetAddress.h"
#include "SocketsOps.h"

#include <strings.h> //bzero
#include <assert.h>
#include <netdb.h>  //gethostbyname_r

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
    bzero(addr_,sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr = ip;
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port)
{
    bzero(&addr_,sizeof(addr_));
    sockets::fromIpPort(ip.c_str(),port,&addr_);
}

std::string InetAddress::toIp() const
{
    char buf[64]="";
    sockets::toIp(buf,sizeof(buf),&addr_);
    return buf;
}

uint16_t InetAddress::toPort() const
{
        return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const
{
    char buf[64]="";
    sockets::toIpPort(buf,sizeof(buf),&addr_);
    return buf;
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(const std::string &hostname, InetAddress *result)
{
    assert(result != NULL);

    struct hostent hent;
    struct hostent *he = NULL;
    int herrno = 0;
    bzero(&hent,sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof(t_resolveBuffer), &he, &herrno);

    if(ret == 0 && he != NULL){
        assert(he->h_addrtype = AF_INET && he->h_length == sizeof(uint32_t));
        result->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr_list[0]);
        return true;
    }
    else
    {
        if(ret)
        {
            //TODO:: LOG_SYSERR << "InetAddress::resolve";

        }
        return false;
    }


}

