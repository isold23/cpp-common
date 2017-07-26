#include "tcp_stream.h"
#include "libnet.h"

CTcpStream::CTcpStream(int liTimeout)
    : miSocket(0)
{
    miSocket = CreateSocket();
    
    if(miSocket > 0) {
        SetSocketRecvTimeout(miSocket, liTimeout);
    }
}

CTcpStream::~CTcpStream(void)
{
    if(0 < miSocket) {
        CloseSocket(miSocket);
        miSocket = 0;
    }
}
bool CTcpStream::Connect(const char* apszDestIp, uint16_t ausPort, int liTimeout)
{
    if(miSocket <= 0) {
        return false;
    }
    
    if(0 == liTimeout) {
        return ConnectToSvr(miSocket, apszDestIp, ausPort);
    } else {
        SetNoBlock(miSocket, 1);
        struct sockaddr_in loAddr;
        memset(&loAddr, 0, sizeof(loAddr));
        loAddr.sin_family = AF_INET;
        loAddr.sin_addr.s_addr = inet_addr(apszDestIp);
        loAddr.sin_port = htons(ausPort);
        
        //connect to server
        if(0 == connect(miSocket, (struct sockaddr*)&loAddr, sizeof(loAddr))) {
            return true;
        }
        
        if(!IsBlockingError()) {
            return false;
        }
        
        struct timeval loTimeout;
        
        loTimeout.tv_sec = liTimeout / 1000;
        
        loTimeout.tv_usec = liTimeout % 1000;
        
        fd_set set;
        
        FD_ZERO(&set);
        
        FD_SET(miSocket, &set);
        
        int retval = select(miSocket + 1, NULL, &set, NULL, &loTimeout);
        
        if(retval == -1) {
            return false;
        } else if(retval == 0) {
            return false;
        }
        
        SetNoBlock(miSocket, 0);
        return true;
    }
}
bool CTcpStream::SendData(const char* apData, uint16_t ausDataLen)
{
    return (ausDataLen == send(miSocket, apData, ausDataLen, 0));
}
int CTcpStream::RecvData(char* apBuff, uint16_t ausBuffSize, int liTimeout)
{
    return recv(miSocket, apBuff, ausBuffSize, 0);
}
void CTcpStream::Disconnect()
{
    if(0 < miSocket) {
        CloseSocket(miSocket);
    }
    
    miSocket = 0;
}
