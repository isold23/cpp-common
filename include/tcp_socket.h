/********************************************************************
 * author: isold.wang@gmail.com
*********************************************************************/
#ifndef _NET_SOCKET_H_
#define _NET_SOCKET_H_

#include "include.h"
#include "critical_section.h"
#include "net_pack.h"
#include "list.h"

#define DEF_LOCAL_ADDR "127.0.0.1"
#define DEF_SOCKET_CATCH_LEN (1024*1024*3)
#define RECV_CATCH_LEN (DEF_SOCKET_CATCH_LEN+DEF_BUFFER_LEN+1)
#define SEND_CATCH_LEN (DEF_SOCKET_CATCH_LEN+1)

class STRU_NET_DATA_INFO
{
public:
    STRU_NET_DATA_INFO()
    {
        buffer = NULL;
        length = 0;
    }
    
    ~STRU_NET_DATA_INFO() {}
    
    STRU_NET_DATA_INFO(const STRU_NET_DATA_INFO& info)
    {
        if(this != &info) {
            buffer = info.buffer;
            length = info.length;
        }
    }
    STRU_NET_DATA_INFO& operator = (const STRU_NET_DATA_INFO& info)
    {
        if(this != &info) {
            buffer = info.buffer;
            length = info.length;
        }
        
        return *this;
    }
public:
    char* buffer;
    int length;
};

//网络状态
enum NET_STAT {
    //错误
    TCP_FAIL = FAIL,
    //监听
    COMMON_TCP_LISTEN,
    COMMON_TCP_SYN_SENT,
    COMMON_TCP_SYN_RECEIVED,
    COMMON_TCP_ESTABLISHED,
    COMMON_TCP_CONNECTING,
    COMMON_TCP_CONNECTED,
    COMMON_AUTHING,
    COMMON_AUTHED,
    COMMON_TCP_CLOSED
};

class CNetSocket
{
public:
    CNetSocket();
    CNetSocket(CNetPack* pack)
    {
        ASSERT(pack != NULL);
        m_pNetPack = pack;
    }
    ~CNetSocket();
    
    inline void SetNetPack(CNetPack* pack)
    {
        ASSERT(pack != NULL);
        m_pNetPack = pack;
        
        //如果不是客户、监听socket，则认为是accept进来的socket，使用epoll默认的协议
        if(miSocket > 0 && !mbClientSocket && !mbListenSocket) {
            mp_net_io = m_pNetPack->create_svr_io(miSocket);
            mp_net_io->open();
        }
    }
    
    //创建socket，并绑定到指定端口
    //参数数据为本地字节序
    bool CreateSocket(const char* ip, const short port);
    //创建socket，不需要绑定
    bool CreateSocket(void);
    //关闭套接字，并清除相关资源
    bool Close(int flag = 2);
    //只是关闭套接字并不清除资源
    void CloseWR(int flag = 2);
    //启动监听
    bool Listen();
    //本地字节序
    bool ConnectServer(const char* ip, const short port);
    //ACCEPT ip 和 port均为本地字节序
    int Accept(uint32_t& ip, uint16_t& port);
    bool SetNoBlock();
    int SendData(const char* buffer, const int length);
    int SendData();
    bool RecvData(char* buffer, int& length);
    bool RecvData();
    const int GetSocket()
    {
        return miSocket;
    }
    
private:
    int _SendData(const char* buffer, const int length);
    int SendCacheData(void);
    //nKeepAlive-是否开启存活
    //nKeepIdle-多长时间无数据收发, 则探测
    //nKeepInterval-探测发包时间间隔
    //nKeepCnt-探测尝试次数
    int SetTcpSockKeepAlive(int nKeepAlive, int nKeepIdle,
                            int nKeepInterval, int nKeepCnt);
                            
public:
    NET_STAT moNetStat;
    int miSocket;
    bool mbListenSocket;
    bool mbClientSocket;
    bool mbCanSend;
    CNetPack* m_pNetPack;
    //io接口
    i_net_io* mp_net_io;
    
private:
    char mszResendBuffer[SEND_CATCH_LEN];
    CCriticalSection moSendSection;
    CCriticalSection moRecvSection;
    int miResendLength;
    _List<STRU_NET_DATA_INFO> moSendList;
    _List<STRU_NET_DATA_INFO> moRecvList;
    char mszRecvCache[RECV_CATCH_LEN];
    uint32_t miRecvCacheLength;
};
#endif //_NET_SOCKET_H_

















