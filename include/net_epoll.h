#ifndef _NET_EPOLL_H_
#define _NET_EPOLL_H_

#include <sys/epoll.h>

#include "include.h"
#include "critical_section.h"
#include "tcp_socket.h"
#include "sigslot.h"

typedef std::map<int, CNetSocket*> NET_SOCKET_LIST;
typedef std::map<int, CNetSocket*>::iterator NET_SOCKET_LIST_ITER;

#define DEF_EPOLL_SIZE 10240
#define DEF_EPOLL_TIMEOUT 0

class CNetEpoll
{
public:
    CNetEpoll();
    CNetEpoll(CNetPack* pack)
    {
        ASSERT(pack != NULL);
        m_pNetPack = pack;
    }
    ~CNetEpoll();
    
    inline void SetPack(CNetPack* pack)
    {
        ASSERT(pack != NULL);
        m_pNetPack = pack;
    }
    
    bool Init(uint32_t aiMaxSocketSize = DEF_EPOLL_SIZE);
    bool Destroy();
    
    int CheckEpollEvent(int time_out = 0);
    int ProcessEpollEvent(int aiEventSize);
    
    bool SendData(int fd, const char* buffer, const int length);
    bool SendAllData(const char* buffer, const int length);
    
    bool Addfd(CNetSocket* apNetSocket);
    bool Delfd(CNetSocket* apNetSocket);
    bool Addfd(int fd);
    bool Delfd(int fd);
    bool Findfd(int fd);
    
    uint32_t GetConnectedSize();
    
    void Dump();
    
private:
    bool Delfd(NET_SOCKET_LIST_ITER iter);
    int AddEpollEvent(CNetSocket* pNetSocket, uint32_t ulEvent);
    int ModifyEpollEvent(CNetSocket* pNetSocket, uint32_t ulEvent);
    int DelEpollEvent(CNetSocket* pNetSocket);
    int DealEpollEvent(int i);
    
public:
    sigslot::signal3<int, char*, int> RecvFrom;
    sigslot::signal1<int> OnErrorNotice;
    sigslot::signal1<int> OnAcceptNotice;
    bool mbHasListenFd;
    bool mbKeepAlive;
private:
    NET_SOCKET_LIST moNetSocketList;
    CCriticalSection moFdSection;
    int miEpfd;
    struct epoll_event* mstruEvent;
    uint32_t miMaxFdNumber;
    CNetPack* m_pNetPack;
};

#endif //_NET_EPOLL_H_






