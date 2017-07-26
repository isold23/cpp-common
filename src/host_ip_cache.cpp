#include "host_ip_cache.h"

CHostIpCache::CHostIpCache(void)
{
}

CHostIpCache::~CHostIpCache(void)
{
}

void CHostIpCache::SetHostIp(const std::string& astrHost, const std::string& astrIp)
{
    CAutoLock loLock(moMutex);
    mmapHostIps[astrHost] = astrIp;
}
bool CHostIpCache::DelHostIp(const std::string& astrHost)
{
    CAutoLock loLock(moMutex);
    std::map<std::string, std::string>::iterator lPos = mmapHostIps.find(astrHost);
    
    if(mmapHostIps.end() != lPos) {
        mmapHostIps.erase(lPos);
        return true;
    }
    
    return false;
}

bool CHostIpCache::GetHostIp(const std::string& astrHost, std::string& astrIp)
{
    CAutoLock loLock(moMutex);
    std::map<std::string, std::string>::iterator lPos = mmapHostIps.find(astrHost);
    
    if(mmapHostIps.end() != lPos) {
        astrIp = lPos->second;
        return true;
    }
    
    return false;
}


