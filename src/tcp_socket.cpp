/********************************************************************
author: isold.wang@gmail.com
purpose: tcp socket
*********************************************************************/
#include "tcp_socket.h"

CNetSocket::CNetSocket()
{
    miSocket = -1;
    memset(mszResendBuffer, 0, SEND_CATCH_LEN);
    miResendLength = 0;
    memset(mszRecvCache, 0, RECV_CATCH_LEN);
    miRecvCacheLength = 0;
    mbCanSend = true;
    moNetStat = COMMON_TCP_CLOSED;
    mbListenSocket = false;
    mbClientSocket = false;
    m_pNetPack = NULL;
    mp_net_io = NULL;
}
CNetSocket::~CNetSocket()
{
    Close();
}
bool CNetSocket::CreateSocket(void)
{
    miSocket = socket(AF_INET, SOCK_STREAM, 0);
    int err = errno;
    
    if(miSocket < 0) {
        TRACE(1, "create socket failed. errno = " << err);
        return false;
    }
    
    return true;
}
bool CNetSocket::CreateSocket(const char* ip, const short port)
{
    miSocket = socket(AF_INET, SOCK_STREAM, 0);
    int err = errno;
    
    if(miSocket < 0) {
        TRACE(1, "CNetSocket::CreateSocket socket() 失败。errno = " << err);
        return false;
    }
    
    //设置地址复用
    int iReuseAddr = 1;
    setsockopt(miSocket, SOL_SOCKET, SO_REUSEADDR, (void*)(& (iReuseAddr))
               , sizeof(iReuseAddr));
    struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    
    if(ip) {
        stAddr.sin_addr.s_addr = inet_addr(ip);
    } else {
        stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    stAddr.sin_port = htons(port);
    socklen_t loinlen = sizeof(sockaddr_in);
    int ret = 0;
    ret = bind(miSocket, (struct sockaddr*)&stAddr, loinlen);
    
    if(ret < 0) {
        err = errno;
        TRACE(1, "CNetSocket::CreateSocket bind() 失败。port = " << port <<
              " ip = " << ip << " errno = " << err);
        close(miSocket);
        return false;
    }
    
    return true;
}
bool CNetSocket::Close(int flag /* = 2 */)
{
    //清除发送缓冲区
    memset(mszResendBuffer, 0, SEND_CATCH_LEN);
    miResendLength = 0;
    //清除发送队列
    CAutoLock send_lock(moSendSection);
    STRU_NET_DATA_INFO loNetDataInfo;
    
    while(moSendList.PopFront(loNetDataInfo)) {
        delete loNetDataInfo.buffer;
        loNetDataInfo.buffer = NULL;
        loNetDataInfo.length = 0;
    }
    
    //清除接收缓冲区
    memset(mszRecvCache, 0,  RECV_CATCH_LEN);
    miRecvCacheLength = 0;
    //清除接收队列
    CAutoLock recv_lock(moRecvSection);
    
    while(moRecvList.PopFront(loNetDataInfo)) {
        delete loNetDataInfo.buffer;
        loNetDataInfo.buffer = NULL;
        loNetDataInfo.length = 0;
    }
    
    if(miSocket != -1) {
        shutdown(miSocket, flag);
        close(miSocket);
        TRACE(1, "CNetSocket::Close 关闭SOCKET: " << miSocket);
        moNetStat = COMMON_TCP_CLOSED;
        miSocket = -1;
    }
    
    return true;
}

void CNetSocket::CloseWR(int flag /* = 2 */)
{
    shutdown(miSocket, flag);
    close(miSocket);
    
    if(mp_net_io) {
        mp_net_io->release();
        mp_net_io = NULL;
    }
}

bool CNetSocket::Listen()
{
    if(listen(miSocket, 1024) < 0) {
        int err = errno;
        TRACE(1, "CNetSocket::Listen 失败。errno = " << err);
        return false;
    }
    
    moNetStat = COMMON_TCP_LISTEN;
    return true;
}
bool CNetSocket::ConnectServer(const char* ip, const short port)
{
    struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    
    if(ip) {
        stAddr.sin_addr.s_addr = inet_addr(ip);
    } else {
        stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    stAddr.sin_port = htons(port);
    socklen_t loinlen = sizeof(sockaddr_in);
    
    if(connect(miSocket, (const struct sockaddr*)&stAddr, loinlen) < 0) {
        int err = errno;
        
        if(errno != EINPROGRESS) {
            TRACE(1, "CNetSocket::ConnectServer 失败。 errno = " << err <<
                  " ip = " << ip << " port = " << port);
            // 出现其他连接错误，则关闭连接
            return false;
        } else {
            // 非阻塞模式操作未完成
            TRACE(1, "CNetSocket::ConnectServer 非阻塞模式操作未完成。");
            moNetStat = COMMON_TCP_CONNECTING;
            return true;
        }
    }
    
    moNetStat = COMMON_TCP_CONNECTED;
    mp_net_io = m_pNetPack->create_clt_io(miSocket);
    mp_net_io->open();
    return true;
}
int CNetSocket::Accept(uint32_t& ip, uint16_t& port)
{
    int err = 0;
    sockaddr_in loSockAddr;
    int nRet = 0;
    socklen_t loinlen = sizeof(sockaddr_in);
    nRet = accept(miSocket, (struct sockaddr*)&loSockAddr, &loinlen);
    
    if(-1 == nRet) {
        err = errno;
        
        if(err == EAGAIN || err == EWOULDBLOCK) {
            TRACE(3, "CNetSocket::Accept accept not ready.");
        } else {
            TRACE(1, "CNetSocket::Accept 错误。errno = " << err);
        }
        
        return 0;
    }
    
    ip = ntohl(loSockAddr.sin_addr.s_addr);
    port = ntohs(loSockAddr.sin_port);
    TRACE(3, "CNetSocket::Accept IP = " << inet_ntoa(loSockAddr.sin_addr) << " PORT = " << port << " fd = " << nRet);
    return nRet;
}
bool CNetSocket::SetNoBlock()
{
    int iFlags = 1; // nonblock reusaddr
    ioctl(miSocket, FIONBIO, &iFlags);
    iFlags = fcntl(miSocket, F_GETFL, 0);
    int nRet = fcntl(miSocket, F_SETFL, iFlags | O_NONBLOCK | O_NDELAY);
    
    if(nRet == -1) {
        TRACE(1, "CNetSocket::SetNoBlock 设置非阻塞属性失败。socket : " << miSocket);
        return false;
    }
    
    int iBufLen = 2 * DEF_SOCKET_CATCH_LEN;
    
    //设置TCP的接收缓冲
    if(setsockopt(miSocket, SOL_SOCKET, SO_RCVBUF, (char*)&iBufLen, sizeof(iBufLen)) == -1) {
        TRACE(1, "CNetSocket::SetNoBlock 设置接收缓冲区失败。socket : " << miSocket);
        return false;
    }
    
    //设置TCP的发送缓冲
    if(setsockopt(miSocket, SOL_SOCKET, SO_SNDBUF, (char*)&iBufLen, sizeof(iBufLen)) == -1) {
        TRACE(1, "CNetSocket::SetNoBlock 设置发送缓冲区失败。socket : " << miSocket);
        return false;
    }
    
    //设置存活
    SetTcpSockKeepAlive(1, 120, 30, 3);
    return true;
}
int CNetSocket::SetTcpSockKeepAlive(int nKeepAlive, int nKeepIdle, int nKeepInterval, int nKeepCnt)
{
    if(setsockopt(miSocket, SOL_SOCKET, SO_KEEPALIVE, (void*)&nKeepAlive, sizeof(nKeepAlive))) {
        TRACE(1, "CNetSocket::SetTcpSockKeepAlive SO_KEEPALIVE 失败。");
        return -1;
    }
    
    if(nKeepAlive) {
        if(nKeepIdle > 0) {
            if(setsockopt(miSocket, SOL_TCP, TCP_KEEPIDLE, (void*)&nKeepIdle, sizeof(nKeepIdle))) {
                TRACE(1, "CNetSocket::SetTcpSockKeepAlive TCP_KEEPIDLE 失败。");
                return -2;
            }
        }
        
        if(nKeepInterval > 0) {
            if(setsockopt(miSocket, SOL_TCP, TCP_KEEPINTVL, (void*)&nKeepInterval, sizeof(nKeepInterval))) {
                TRACE(1, "CNetSocket::SetTcpSockKeepAlive TCP_KEEPINTVL 失败。");
                return -3;
            }
        }
        
        if(nKeepCnt > 0) {
            if(setsockopt(miSocket, SOL_TCP, TCP_KEEPCNT, (void*)&nKeepCnt, sizeof(nKeepCnt))) {
                TRACE(1, "CNetSocket::SetTcpSockKeepAlive TCP_KEEPCNT 失败。");
                return -4;
            }
        }
    }
    
    return 0;
}
int CNetSocket::SendData(const char* buffer, const int length)
{
    ASSERT(NULL != m_pNetPack);
    char lszSendBuffer[m_pNetPack->_max_pack_size];
    memset(lszSendBuffer, 0, m_pNetPack->_max_pack_size);
    uint32_t liSendLen = m_pNetPack->_max_pack_size;
    int nRet = m_pNetPack->Pack(buffer, length, lszSendBuffer, liSendLen);
    
    if(nRet < 0) {
        TRACE(1, "CNetSocket::SendData pack error. fd = " << miSocket << " ret = " << nRet);
        return -1;
    }
    
    int ssize = 0;
    memcpy(&ssize, lszSendBuffer, 4);
    TRACE(5, "send size : " << ssize);
    CAutoLock lock(moSendSection);
    
    if(((liSendLen + miResendLength) <= DEF_SOCKET_CATCH_LEN) && moSendList.IsEmpty()) {
        memcpy(mszResendBuffer + miResendLength, lszSendBuffer, liSendLen);
        miResendLength += liSendLen;
    } else {
        STRU_NET_DATA_INFO struNetDataInfo;
        struNetDataInfo.buffer =  new char[liSendLen];
        memset(struNetDataInfo.buffer, 0, liSendLen);
        memcpy(struNetDataInfo.buffer, lszSendBuffer, liSendLen);
        struNetDataInfo.length = liSendLen;
        moSendList.PushBack(struNetDataInfo);
    }
    
    return 1;
}
/************************************************************************
返回值：
1   发送数据阻塞，等待下次事件通知，  需要设置socket的属性
0   函数正常返回
-1  网络连接出现问题，需要断开该连接
************************************************************************/
int CNetSocket::SendCacheData()
{
    if(miResendLength != 0) {
        int nRet = 0;
        nRet = _SendData(mszResendBuffer, miResendLength);
        TRACE(5, "send data length : " << nRet);
        
        if(nRet == 0) {
            mbCanSend = false;
            return 1;
        }
        
        if(nRet < 0) {
            TRACE(1, "CNetSocket::SendCacheData nRet = " << nRet << " miResendLength = " << miResendLength);
            return -1;
        }
        
        if((nRet < miResendLength) && (nRet > 0)) {
            miResendLength -= nRet;
            ASSERT(miResendLength > 0);
            memmove(mszResendBuffer, mszResendBuffer + nRet, miResendLength + 1);
            mbCanSend = false;
            return 1;
        }
        
        if(nRet > miResendLength) {
            mbCanSend = false;
            TRACE(1, "CNetSocket::SendCacheData nRet = " << nRet << " miResendLength = " << miResendLength);
            return -1;
        }
    }
    
    miResendLength = 0;
    memset(mszResendBuffer, 0, SEND_CATCH_LEN);
    return 0;
}

/************************************************************************
返回值：
-1  网络连接出现问题，需要断开该连接
0    函数正常退出
1     发送数据阻塞，等待下次事件通知，  需要设置socket的属性
2   没有数据可以发送
3   网络连接不可以发送，直接返回
************************************************************************/
int CNetSocket::SendData()
{
    if(!mbCanSend) {
        return 3;
    }
    
    CAutoLock lock(moSendSection);
    
    if(0 == miResendLength && moSendList.IsEmpty()) {
        return 2;
    }
    
    int nSendCacheRtn = SendCacheData();
    
    if(0 != nSendCacheRtn) {
        return nSendCacheRtn;
    }
    
    STRU_NET_DATA_INFO loNetDataInfo;
    
    //如果将所有数据发送完毕就用while
    while(moSendList.TryPop(loNetDataInfo)) {
        if(miResendLength + loNetDataInfo.length <= DEF_SOCKET_CATCH_LEN) {
            STRU_NET_DATA_INFO info;
            moSendList.PopFront(info);
            ASSERT(info.length == loNetDataInfo.length);
            ASSERT(info.buffer == loNetDataInfo.buffer);
            memcpy(mszResendBuffer + miResendLength, loNetDataInfo.buffer, loNetDataInfo.length);
            miResendLength += loNetDataInfo.length;
            delete [] loNetDataInfo.buffer;
            loNetDataInfo.buffer = NULL;
            loNetDataInfo.length = 0;
        } else {
            int liReSendCache = SendCacheData();
            
            if(0 != liReSendCache) {
                TRACE(1, "CNetSocket::SendData 非正常结束发送socket 数据1。fd : " << miSocket);
                return liReSendCache;
            }
        }
    }
    
    int iRtn = SendCacheData();
    
    if(0 != iRtn) {
        TRACE(1, "CNetSocket::SendData 非正常结束发送socket 数据2。fd : " << miSocket);
        return iRtn;
    }
    
    return 0;
}

/************************************************************************
返回值：
>0 发送的字节数
0    网络出现阻塞，需要等待下次通知
-1  网络连接出现异常
************************************************************************/
int CNetSocket::_SendData(const char* buffer, const int length)
{
    int nRet = mp_net_io->write(miSocket, buffer, length);
    int err = errno;
    int size = 0;
    memcpy(&size, buffer, 4);
    TRACE(5, "size : " << size);
    
    if(nRet > 0) {
        return nRet;
    } else if(nRet < 0) {
        if((err == EAGAIN) || (err == EWOULDBLOCK) || (EINTR == err)) {
            return 0;
        } else {
            TRACE(1, "CNetSocket::_SendData 发送数据失败。errno = " << err);
            return -1;
        }
    } else if(nRet == 0) {
        TRACE(1, "CNetSocket::_SendData 发送数据失败。errno = " << err);
        return -1;
    }
    
    return 0;
}
bool CNetSocket::RecvData(char* buffer, int& length)
{
    ASSERT(buffer != NULL);
    CAutoLock lock(moRecvSection);
    
    if(moRecvList.IsEmpty())
        return false;
        
    STRU_NET_DATA_INFO loDataInfo;
    moRecvList.PopFront(loDataInfo);
    memcpy(buffer, loDataInfo.buffer, loDataInfo.length);
    length = loDataInfo.length;
    delete [] loDataInfo.buffer;
    loDataInfo.buffer = NULL;
    return true;
}
bool CNetSocket::RecvData()
{
    char lszBuffer[DEF_SOCKET_CATCH_LEN];
    int liLength = DEF_SOCKET_CATCH_LEN;
    int err = 0;
    CAutoLock lock(moRecvSection);
    
    while(moNetStat == COMMON_TCP_ESTABLISHED || moNetStat == COMMON_TCP_CONNECTED) {
        memset(lszBuffer, 0, DEF_SOCKET_CATCH_LEN);
        liLength = DEF_SOCKET_CATCH_LEN;
        int nRet = mp_net_io->read(miSocket, lszBuffer, liLength);
        err = errno;
        
        if(nRet > 0) {
            ASSERT((miRecvCacheLength + nRet) <= (RECV_CATCH_LEN - 1));
            memcpy(mszRecvCache + miRecvCacheLength, lszBuffer, nRet);
            miRecvCacheLength += nRet;
            
            while(miRecvCacheLength > m_pNetPack->_min_pack_size) {
                char lszRecvBuffer[m_pNetPack->_max_pack_size];
                memset(lszRecvBuffer, 0, m_pNetPack->_max_pack_size);
                uint32_t liRecvBufferLen = m_pNetPack->_max_pack_size;
                uint32_t liRecvDataLen = 0;
                int ret = m_pNetPack->Unpack((const char*)mszRecvCache, miRecvCacheLength, lszRecvBuffer, liRecvBufferLen, liRecvDataLen);
                
                if(ret > 0) {
                    STRU_NET_DATA_INFO loNetDataInfo;
                    char* lpBuffer = new char[liRecvBufferLen + 1];
                    memset(lpBuffer, 0, liRecvBufferLen + 1);
                    memcpy(lpBuffer, lszRecvBuffer, liRecvBufferLen);
                    loNetDataInfo.length = liRecvBufferLen;
                    loNetDataInfo.buffer = lpBuffer;
                    moRecvList.PushBack(loNetDataInfo);
                    miRecvCacheLength -= liRecvDataLen;
                    assert(miRecvCacheLength >= 0);
                    
                    if(miRecvCacheLength > 0) {
                        memmove(mszRecvCache, mszRecvCache + liRecvDataLen, miRecvCacheLength + 1);
                    } else {
                        memset(mszRecvCache, 0, RECV_CATCH_LEN);
                    }
                } else if(ret == 0) {
                    break;
                } else {
                    TRACE(1, "CNetSocket::RecvData unpack failed, ret : " << ret);
                    return false;
                }
            }
            
            if(nRet >= DEF_SOCKET_CATCH_LEN) {
                continue;
            } else {
                return true;
            }
        } else if(nRet == 0) {
            TRACE(1, "CNetSocket::RecvData recv errno : " << err << " fd = " << miSocket);
            return false;
        } else if((nRet < 0) && (errno != ECONNRESET)) {
            return true;
        }
    }
    
    TRACE(1, "CNetSocket::RecvData return recv errno : " << err << " fd = " << miSocket);
    return false;
}



















