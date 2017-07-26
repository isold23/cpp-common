#ifndef _HOST_IP_CACHE_H_
#define _HOST_IP_CACHE_H_

#include <string>
#include <map>
#include "critical_section.h"

class CHostIpCache
{
public:
    CHostIpCache(void);
    ~CHostIpCache(void);
    void SetHostIp(const std::string& astrHost, const std::string& astrIp);
    bool DelHostIp(const std::string& astrHost);
    bool GetHostIp(const std::string& astrHost, std::string& astrIp);
private:
    std::map<std::string, std::string> mmapHostIps;
    CCriticalSection moMutex;
};
#endif //_HOST_IP_CACHE_H_
