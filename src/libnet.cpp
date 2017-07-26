#include "libnet.h"

#ifdef WIN32
#pragma comment(lib,"Ws2_32.lib")
#endif

std::string GetAddrStr(const sockaddr_in& aoAddr)
{
    char lszBuff[32] = {0};
    sprintf(lszBuff, "%s:%d", inet_ntoa(aoAddr.sin_addr), ntohs(aoAddr.sin_port));
    return lszBuff;
}

//check ip whether local ip
bool IsLocalIp(const char* apIpAddr)
{
    if(strcmp("127.0.0.1", apIpAddr) == 0
            || strncmp("10.218", apIpAddr, 6) == 0
            || strncmp("10.210", apIpAddr, 6) == 0) {
        return true;
    }
    
    return false;
}

bool InitSocket()
{
#ifdef WIN32
    WSADATA wsaData = {0};
    return 0 == WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
    return true;
#endif
}
void FiniSocket()
{
#ifdef WIN32
    WSACleanup();
#endif
}

int CreateSocket(int type)
{
    if(SOCK_STREAM == type) {
        return (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    
    if(SOCK_STREAM == type) {
        return (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    
    return 0;
}
void CloseSocket(int aiSocket)
{
#ifdef WIN32
    closesocket(aiSocket);
#else
    close(aiSocket);
#endif
}
bool Bind(int aiSocket, const char* apszAddr, uint16_t ausPort)
{
    struct sockaddr_in loAddr;
    memset(&loAddr, 0, sizeof(loAddr));
    loAddr.sin_family = AF_INET;
    
    if(strcmp("", apszAddr) != 0) {
        loAddr.sin_addr.s_addr = inet_addr(apszAddr);
    } else {
        loAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    loAddr.sin_port = htons(ausPort);
    return 0 == bind(aiSocket, (struct sockaddr*)&loAddr, sizeof(loAddr));
}
bool Listen(int aiSocket, int aiBacklog)
{
    return 0 == listen(aiSocket, aiBacklog);
}
bool SetNoBlock(int aiSocket, int aiNoBlock)
{
#ifdef WIN32
    unsigned long lulMode = aiNoBlock;
    return 0 == ioctlsocket(aiSocket, FIONBIO, &lulMode);
#else
    int iFlags = aiNoBlock; // nonblock reusaddr
    ioctl(aiSocket, FIONBIO, &iFlags);
    iFlags = fcntl(aiSocket, F_GETFL, 0);
    
    if(1 == aiNoBlock) {
        fcntl(aiSocket, F_SETFL, iFlags | O_NONBLOCK | O_NDELAY);
    } else {
        fcntl(aiSocket, F_SETFL, iFlags | ~O_NONBLOCK | ~O_NDELAY);
    }
    
    return true;
#endif
}
bool SetSocketBuffer(int aiSocket, int aiRecvBuffSize, int aiSendBuffSize)
{
    //设置TCP的接收缓冲
    return 0 == setsockopt(aiSocket, SOL_SOCKET, SO_RCVBUF, (char*)&aiRecvBuffSize, sizeof(aiRecvBuffSize))
           && 0 == setsockopt(aiSocket, SOL_SOCKET, SO_SNDBUF, (char*)&aiSendBuffSize, sizeof(aiSendBuffSize));
}

bool SetReuseAddr(int aiSocket)
{
    int iReuseAddr = 1;
    return 0 == setsockopt(aiSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&iReuseAddr, sizeof(iReuseAddr));
}

bool SetSocketRecvTimeout(int aiSocket, int aiTimeout)
{
#ifdef WIN32
    int   timeout = aiTimeout;
    setsockopt(aiSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
    struct timeval loTimeout;
    loTimeout.tv_sec = aiTimeout / 1000;
    loTimeout.tv_usec = aiTimeout % 1000;
    setsockopt(aiSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&loTimeout, sizeof(loTimeout));
#endif
    return true;
}

bool ConnectToSvr(int aiSocket, const char* apszAddr, uint16_t ausPort)
{
    //set svr addr
    struct sockaddr_in loAddr;
    memset(&loAddr, 0, sizeof(loAddr));
    loAddr.sin_family = AF_INET;
    loAddr.sin_addr.s_addr = inet_addr(apszAddr);
    loAddr.sin_port = htons(ausPort);
    
    //connect to server
    if(0 != connect(aiSocket, (struct sockaddr*)&loAddr, sizeof(loAddr))) {
        return false;
    }
    
    return true;
}
bool CreateTcpServer(int aiSocket, const char* apszAddr, uint16_t ausPort,
                     bool abReuseAddr, bool abNoBlock, int aiRecvBuffSize, int aiSendBuffSize)
{
    //set options
    if(abReuseAddr) {
        if(!SetReuseAddr(aiSocket)) {
            //TRACE(LOG_ERROR,"set reuse addr fail,socket:"<<aiSocket);
            return false;
        }
    }
    
    if(abNoBlock) {
        if(!SetNoBlock(aiSocket)) {
            //TRACE(LOG_ERROR,"set no block fail,socket:"<<aiSocket);
            return false;
        }
    }
    
    if(0 < aiSendBuffSize && 0 < aiRecvBuffSize) {
        if(!SetSocketBuffer(aiSocket, aiRecvBuffSize, aiSendBuffSize)) {
            //TRACE(LOG_ERROR,"set socket send & recv buff fail,socket:"<<aiSocket);
            return false;
        }
    }
    
    //bind addr
    if(!Bind(aiSocket, apszAddr, ausPort)) {
        //TRACE(LOG_ERROR,"bind to addr:"<<apszSvrAddr<<":"<<ausSvrPort<<" fail!");
        return false;
    }
    
    //listen on created socket
    if(!Listen(aiSocket)) {
        //TRACE(LOG_ERROR,"listen socket fail,socket:"<<aiSocket);
        return false;
    }
    
    return true;
}

//create sync socket with recv timeout
int CreateSyncSocket(int aiRecvTimeout)
{
    int liSocket = CreateSocket();
    
    if(0 < liSocket) {
#ifdef WIN32
        int   timeout = aiRecvTimeout;
        setsockopt(liSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        struct timeval loTimeout;
        loTimeout.tv_sec = aiRecvTimeout / 1000;
        loTimeout.tv_usec = aiRecvTimeout % 1000;
        setsockopt(liSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&loTimeout, sizeof(loTimeout));
#endif
    }
    
    return liSocket;
}

int GetNetError()
{
#ifdef WIN32
    return ::WSAGetLastError();
#else
    return errno;
#endif
}

bool IsBlockingError(int aiError)
{
    if(0 == aiError) {
        aiError = GetNetError();
    }
    
#ifdef WIN32
    return (WSAEWOULDBLOCK == aiError);
#else
    return (EAGAIN == aiError) || (EINPROGRESS == aiError);
#endif
}

void SetMaxOpenFiles(uint32_t auiMaxFileNums)
{
#ifndef WIN32
    //修改core文件大小限制和进程最大文件句柄数
    struct rlimit rlimit_file_new;
    struct rlimit rlimit_file_old;
    
    if(getrlimit(RLIMIT_NOFILE, &rlimit_file_old) == 0) {
        rlimit_file_new.rlim_cur = auiMaxFileNums;
        rlimit_file_new.rlim_max = auiMaxFileNums;
        
        if(setrlimit(RLIMIT_NOFILE, &rlimit_file_new) != 0) {
            int errorcode = errno;
            
            if(errorcode == EFAULT) {
                return;
            } else if(errorcode == EINVAL) {
                return;
            } else if(errorcode == EPERM) {
                return;
            }
            
            rlimit_file_new.rlim_cur = rlimit_file_old.rlim_cur;
            rlimit_file_new.rlim_max = rlimit_file_old.rlim_max;
            setrlimit(RLIMIT_NOFILE, &rlimit_file_new);
        }
    }
    
#endif
}

/*!
* 功能: 设置TcpSocket的keepalive相关时间间隔值
* 参数: nSocket-要设置的Socket id, nKeepAlive-设置或者清除keepalive属性
* @n    nKeepIdle-多长时间无数据收发, 则探测, nKeepInterval-探测发包时间间隔
* @n    nKeepCnt-探测尝试次数
* @n作者: huangjun
* @n日期: 2009-01-06
*/
int SetTcpSockKeepAlive(int nSocket, int nKeepAlive, int nKeepIdle, int nKeepInterval, int nKeepCnt)
{
#ifdef WIN32

    if(setsockopt(nSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&nKeepAlive, sizeof(nKeepAlive))) {
        return -1;
    }
    
    if(nKeepAlive) {
        DWORD dwError = 0L ;
        DWORD ldwRetBytes = 0;
        tcp_keepalive sKA_Settings = {0}, sReturned = {0} ;
        sKA_Settings.onoff = 1 ;
        sKA_Settings.keepalivetime = nKeepIdle * 1000 ;
        sKA_Settings.keepaliveinterval = nKeepInterval * 1000 ;
        
        if(WSAIoctl(nSocket, SIO_KEEPALIVE_VALS, &sKA_Settings,
                    sizeof(sKA_Settings), NULL, 0, &ldwRetBytes,
                    NULL, NULL) != 0) {
            dwError = WSAGetLastError() ;
        }
    }
    
#else
    
    if(setsockopt(nSocket, SOL_SOCKET, SO_KEEPALIVE, (void*)&nKeepAlive, sizeof(nKeepAlive))) {
        return -1;
    }
    
    if(nKeepAlive) {
        if(nKeepIdle > 0) {
            if(setsockopt(nSocket, SOL_TCP, TCP_KEEPIDLE, (void*)&nKeepIdle, sizeof(nKeepIdle))) {
                return -2;
            }
        }
    
        if(nKeepInterval > 0) {
            if(setsockopt(nSocket, SOL_TCP, TCP_KEEPINTVL, (void*)&nKeepInterval, sizeof(nKeepInterval))) {
                return -3;
            }
        }
    
        if(nKeepCnt > 0) {
            if(setsockopt(nSocket, SOL_TCP, TCP_KEEPCNT, (void*)&nKeepCnt, sizeof(nKeepCnt))) {
                return -4;
            }
        }
    }
    
#endif
    return 0;
}

//根据host获取ip
bool GetIpByHostName(const std::string& astrHost, std::string& astrIp)
{
    hostent* pHostInfo = NULL;
#ifndef WIN32
    hostent hostinfo;
    char lszBuff[8192] = {0};
    int liRet = 0;
    
    if(0 != gethostbyname_r(astrHost.c_str(), &hostinfo, lszBuff, 8192, &pHostInfo, &liRet)) {
        return false;
    }
    
#else
    pHostInfo = gethostbyname(astrHost.c_str());
#endif
    
    if(NULL == pHostInfo) {
        return false;
    }
    
    char* lpszTemp = inet_ntoa(* (in_addr*) * (pHostInfo->h_addr_list));
    
    if(NULL == lpszTemp) {
        return false;
    }
    
    astrIp = lpszTemp;
    return true;
}

//get peer addr
bool GetPeerAddr(int aiSocket, sockaddr_in& aoAddr)
{
#ifdef WIN32
    int liAddrSize = sizeof(aoAddr);
#else
    socklen_t liAddrSize = sizeof(aoAddr);
#endif
    return 0 == getpeername(aiSocket, (sockaddr*)&aoAddr, &liAddrSize);
}