#ifndef _NET_ADDRESS_H_
#define _NET_ADDRESS_H_

#include "include.h"

#define INET_IP_STRLEN   (sizeof("255.255.255.255") - 1)
#define INET_ADDR_STRLEN  (sizeof("255.255.255.255:65535") - 1)

class CNetAddress
{
public:
    inline static char* GetStrIp(uint32_t ip)
    {
        sockaddr_in addr;
        addr.sin_addr.s_addr = htonl(ip);
        return inet_ntoa(addr.sin_addr);
    }
};

#endif //_NET_ADDRESS_H_


