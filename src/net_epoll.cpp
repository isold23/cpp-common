#include "net_epoll.h"

CNetEpoll::CNetEpoll()
{
    miEpfd = -1;
    mbHasListenFd = false;
    mbKeepAlive = true;
    miMaxFdNumber = 10240;
    mstruEvent = NULL;
}

CNetEpoll::~CNetEpoll()
{
}

bool CNetEpoll::Init(uint32_t aiMaxSocketSize /* = DEF_EPOLL_SIZE */)
{
    if(mstruEvent != NULL) {
        delete [] mstruEvent;
        mstruEvent = NULL;
    }
    
    mstruEvent = new epoll_event[aiMaxSocketSize];
    
    if(mstruEvent == NULL) {
        TRACE(1, "CNetEpoll::Init Epoll事件数组分配失败。 ");
        return false;
    }
    
    miEpfd = epoll_create(aiMaxSocketSize);
    
    if(miEpfd == -1) {
        TRACE(1, "CNetEpoll::Init epoll_creat 失败。 ");
        return false;
    }
    
    miMaxFdNumber = aiMaxSocketSize;
    return true;
}

bool CNetEpoll::Destroy()
{
    if(mstruEvent != NULL) {
        delete [] mstruEvent;
        mstruEvent = NULL;
    }
    
    CAutoLock lock(moFdSection);
    
    while(moNetSocketList.size() != 0) {
        NET_SOCKET_LIST_ITER iter = moNetSocketList.begin();
        int fd = iter->first;
        Delfd(fd);
    }
    
    if(miEpfd != -1) {
        close(miEpfd);
    }
    
    return true;
}

bool CNetEpoll::Addfd(CNetSocket* apNetSocket)
{
    ASSERT(apNetSocket != NULL);
    ASSERT(moNetSocketList.end() == moNetSocketList.find(apNetSocket->miSocket));
    CAutoLock lock(moFdSection);
    moNetSocketList[apNetSocket->miSocket] = apNetSocket;
    
    if(AddEpollEvent(apNetSocket, EPOLLIN | EPOLLET) < 0) {
        NET_SOCKET_LIST_ITER iter = moNetSocketList.find(apNetSocket->miSocket);
        
        if(iter != moNetSocketList.end()) {
            moNetSocketList.erase(iter);
        }
        
        TRACE(1, "CNetEpoll::Addfd * 失败。 errno = " << errno);
        return false;
    }
    
    TRACE(5, "CNetEpoll::Addfd fd = " << apNetSocket->miSocket);
    return true;
}

bool CNetEpoll::Addfd(int fd)
{
    ASSERT(moNetSocketList.end() == moNetSocketList.find(fd));
    CAutoLock lock(moFdSection);
    CNetSocket* lpNetSocket = new CNetSocket;
    lpNetSocket->miSocket = fd;
    lpNetSocket->moNetStat = COMMON_TCP_ESTABLISHED;
    lpNetSocket->mbListenSocket = false;
    bool bRet = lpNetSocket->SetNoBlock();
    
    if(!bRet) {
        delete lpNetSocket;
        lpNetSocket = NULL;
        return false;
    }
    
    lpNetSocket->SetNetPack(m_pNetPack);
    moNetSocketList[fd] = lpNetSocket;
    
    if(AddEpollEvent(lpNetSocket, EPOLLIN | EPOLLET) < 0) {
        NET_SOCKET_LIST_ITER iter = moNetSocketList.find(fd);
        
        if(iter != moNetSocketList.end()) {
            moNetSocketList.erase(iter);
        }
        
        TRACE(1, "CNetEpoll::Addfd 失败。 errno = " << errno);
        return false;
    }
    
    return true;
}

bool CNetEpoll::Delfd(int fd)
{
    CAutoLock lock(moFdSection);
    CNetSocket* lpNetSocket = NULL;
    NET_SOCKET_LIST_ITER iter = moNetSocketList.find(fd);
    
    if(iter != moNetSocketList.end()) {
        lpNetSocket = iter->second;
    }
    
    if(NULL == lpNetSocket) {
        return true;
    }
    
    if(DelEpollEvent(lpNetSocket) < 0) {
        TRACE(1, "CNetEpoll::Delfd 失败。 errno = " << errno);
    }
    
    if(!lpNetSocket->mbListenSocket && !lpNetSocket->mbClientSocket) {
        delete lpNetSocket;
        lpNetSocket = NULL;
    }
    
    moNetSocketList.erase(iter);
    return true;
}

bool CNetEpoll::Delfd(CNetSocket* apNetSocket)
{
    ASSERT(apNetSocket != NULL);
    CAutoLock lock(moFdSection);
    
    if(DelEpollEvent(apNetSocket) < 0) {
        TRACE(1, "CNetEpoll::Delfd 失败。 errno = " << errno);
    }
    
    moNetSocketList.erase(apNetSocket->miSocket);
    
    if(!apNetSocket->mbListenSocket && !apNetSocket->mbClientSocket) {
        delete apNetSocket;
        apNetSocket = NULL;
    }
    
    return true;
}

bool CNetEpoll::Delfd(NET_SOCKET_LIST_ITER iter)
{
    CAutoLock lock(moFdSection);
    CNetSocket* lpNetSocket = NULL;
    lpNetSocket = iter->second;
    
    if(NULL == lpNetSocket) {
        return true;
    }
    
    if(DelEpollEvent(lpNetSocket) < 0) {
        TRACE(1, "CNetEpoll::Delfd NET_SOCKET_LIST_ITER 失败。 errno = " << errno);
    }
    
    if(!lpNetSocket->mbListenSocket && !lpNetSocket->mbClientSocket) {
        delete lpNetSocket;
        lpNetSocket = NULL;
    }
    
    moNetSocketList.erase(iter);
    return true;
}

bool CNetEpoll::Findfd(int fd)
{
    CAutoLock lock(moFdSection);
    NET_SOCKET_LIST_ITER iter = moNetSocketList.find(fd);
    
    if(iter != moNetSocketList.end()) {
        return true;
    }
    
    return false;
}

int CNetEpoll::AddEpollEvent(CNetSocket* pNetSocket, uint32_t ulEvent)
{
    struct epoll_event ev;
    ev.data.ptr = pNetSocket;
    ev.events = ulEvent;
    return epoll_ctl(miEpfd, EPOLL_CTL_ADD, pNetSocket->miSocket, &ev);
}

int CNetEpoll::ModifyEpollEvent(CNetSocket* pNetSocket, uint ulEvent)
{
    struct epoll_event ev;
    ev.data.ptr = pNetSocket;
    ev.events = ulEvent;
    
    if(epoll_ctl(miEpfd, EPOLL_CTL_MOD, pNetSocket->miSocket, &ev) < 0) {
        int err = errno;
        
        if(err != ENOENT) {
            return epoll_ctl(miEpfd, EPOLL_CTL_ADD, pNetSocket->miSocket, &ev);
        } else {
            return -1;
        }
    }
    
    return 0;
}

int CNetEpoll::DelEpollEvent(CNetSocket* pNetSocket)
{
    struct epoll_event ev;
    ev.data.ptr = pNetSocket;
    ev.events = 0;
    return epoll_ctl(miEpfd, EPOLL_CTL_DEL, pNetSocket->miSocket, &ev);
}

int CNetEpoll::CheckEpollEvent(int time_out/* =0 */)
{
    ASSERT(mstruEvent != NULL);
    int nRet = 0;
    int err = 0;
    nRet = epoll_wait(miEpfd, mstruEvent, miMaxFdNumber, time_out);
    err = errno;
    
    if(nRet == -1) {
        if(err != errno) {
            TRACE(1, "CNetEpoll::CheckEpollEvent 出错。errno = " << errno);
        }
        
        return 0;
    }
    
    return nRet;
}

int CNetEpoll::DealEpollEvent(int i)
{
    int error_fd = -1;
    bool lbRecvFlag = false;
    CAutoLock lock(moFdSection);
    CNetSocket* lpNetFd  = NULL;
    lpNetFd = (CNetSocket*)mstruEvent[i].data.ptr;
    
    if(moNetSocketList.find(lpNetFd->miSocket) == moNetSocketList.end()) {
        TRACE(1, "CNetEpoll::DealEpollEvent 未查到FD: " << lpNetFd->miSocket);
        return error_fd;
    }
    
    if(lpNetFd->mbListenSocket) {
        if(mstruEvent[i].events & (EPOLLERR | EPOLLHUP)) {
            TRACE(1, "CNetEpoll::DealEpollEvent 监听SOCKET出现错误。errno = " << errno);
            return error_fd;
        }
        
        if(mstruEvent[i].events & EPOLLIN) {
            while(true) {
                uint32_t ip = 0;
                uint16_t port = 0;
                int _fd  =  lpNetFd->Accept(ip, port);
                
                if(0 == _fd) {
                    break;
                } else {
                    Addfd(_fd);
                    uint32_t liCurrentFdNumber = GetConnectedSize();
                    
                    if(liCurrentFdNumber > miMaxFdNumber) {
                        break;
                    }
                    OnAcceptNotice(_fd);
                }
            }
        }
    } else {
        if(mstruEvent[i].events & (EPOLLERR | EPOLLHUP)) {
            TRACE(1, "CNetEpoll::DealEpollEvent  net errno: " << errno
                  << " fd = " << lpNetFd->miSocket);
            int lfd = lpNetFd->miSocket;
            Delfd(lpNetFd->miSocket);
            //OnErrorNotice(lfd);
            error_fd = lfd;
            return error_fd;
        }
        
        if(mstruEvent[i].events & EPOLLIN) {
            if(!lpNetFd->RecvData()) {
                int lfd = lpNetFd->miSocket;
                Delfd(lpNetFd->miSocket);
                error_fd = lfd;
                //OnErrorNotice(lfd);
            } else {
                lbRecvFlag = true;
            }
            
            while(lbRecvFlag) {
                char buffer[DEF_BUFFER_LEN];
                memset(buffer, 0, DEF_BUFFER_LEN);
                int length = DEF_BUFFER_LEN;
                bool lbRet = lpNetFd->RecvData(buffer, length);
                
                if(lbRet) {
                    RecvFrom(lpNetFd->miSocket, buffer, length);
                } else {
                    break;
                }
            }
        }
        
        if(mstruEvent[i].events & EPOLLOUT) {
            lpNetFd->mbCanSend = true;
            ModifyEpollEvent(lpNetFd, EPOLLIN | EPOLLET);
            int nRet = lpNetFd->SendData();
            
            if(-1 == nRet) {
                int lfd = lpNetFd->miSocket;
                Delfd(lpNetFd->miSocket);
                //OnErrorNotice(lfd);
                error_fd = lfd;
            } else if(1 == nRet) {
                ModifyEpollEvent(lpNetFd, EPOLLIN | EPOLLOUT | EPOLLET);
            }
        }
    }
    
    return error_fd;
}
int CNetEpoll::ProcessEpollEvent(int aiEventSize)
{
    for(int i = 0; i < aiEventSize; i++) {
        int nRet = DealEpollEvent(i);
        
        if(nRet >= 0) {
            OnErrorNotice(nRet);
        }
    }
    
    return 0;
}

bool CNetEpoll::SendData(int fd, const char* buffer, const int length)
{
    if(fd <= 0) {
        return false;
    }
    
    CAutoLock lock(moFdSection);
    CNetSocket* lpNetSocket = NULL;
    NET_SOCKET_LIST_ITER iter = moNetSocketList.find(fd);
    
    if(iter != moNetSocketList.end()) {
        lpNetSocket = iter->second;
    }
    
    if(NULL == lpNetSocket) {
        TRACE(1, "CNetEpoll::SendData not found fd, fd :  " << fd);
        return false;
    }
    
    lpNetSocket->SendData(buffer, length);
    int nRet = lpNetSocket->SendData();
    
    if(-1 == nRet) {
        int lfd = lpNetSocket->miSocket;
        Delfd(lpNetSocket->miSocket);
        OnErrorNotice(lfd);
        return false;
    } else if(1 == nRet) {
        ModifyEpollEvent(lpNetSocket, EPOLLIN | EPOLLOUT | EPOLLET);
    }
    
    return true;
}

bool CNetEpoll::SendAllData(const char* buffer, const int length)
{
    CAutoLock lock(moFdSection);
    NET_SOCKET_LIST_ITER iter = moNetSocketList.begin();
    
    for(; iter != moNetSocketList.end();) {
        CNetSocket* lpNetSocket = NULL;
        lpNetSocket = iter->second;
        
        if(NULL == lpNetSocket) {
            iter++;
            continue;
        }
        
        if(!lpNetSocket->mbListenSocket && !lpNetSocket->mbClientSocket) {
            lpNetSocket->SendData(buffer, length);
            int nRet = lpNetSocket->SendData();
            
            if(-1 == nRet) {
                int lfd = lpNetSocket->miSocket;
                Delfd(iter++);
                OnErrorNotice(lfd);
                continue;
            } else if(1 == nRet) {
                ModifyEpollEvent(lpNetSocket, EPOLLIN | EPOLLOUT | EPOLLET);
            }
        }
        
        iter++;
    }
    
    return true;
}

uint32_t CNetEpoll::GetConnectedSize()
{
    CAutoLock lock(moFdSection);
    uint32_t luiSize = moNetSocketList.size();
    return luiSize;
}

void CNetEpoll::Dump()
{
    CAutoLock lock(moFdSection);
    TRACE(2, "CNetEpoll::Dump socket 个数: " << moNetSocketList.size());
    NET_SOCKET_LIST_ITER iter = moNetSocketList.begin();
    
    for(; iter != moNetSocketList.end(); iter++) {
        CNetSocket* lpNetSocket = NULL;
        lpNetSocket = iter->second;
        
        if(NULL == lpNetSocket) {
            TRACE(2, "CNetEpoll::Dump fd : " << iter->first << " socket信息为空。");
        } else {
            TRACE(2, "CNetEpoll::Dump fd : " << iter->first << " 网络连接状态：" << lpNetSocket->moNetStat);
        }
    }
}

