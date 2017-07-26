#ifndef _LIB_NET_H_
#define _LIB_NET_H_
#include "include.h"

std::string GetAddrStr(const sockaddr_in& aoAddr);

//check ip whether local ip
bool IsLocalIp(const char* apIpAddr);

bool InitSocket();
void FiniSocket();
int CreateSocket(int type = SOCK_STREAM);
void CloseSocket(int aiSocket);
bool Bind(int aiSocket, const char* apszAddr, uint16_t ausPort);
bool Listen(int aiSocket, int aiBacklog = 1024);
bool SetNoBlock(int aiSocket, int liNoBlock = 1);
bool SetSocketBuffer(int aiSocket, int aiRecvBuffSize, int aiSendBuffSize);
bool SetReuseAddr(int aiSocket);
bool SetSocketRecvTimeout(int aiSocket, int aiTimeout);
bool ConnectToSvr(int aiSocket, const char* apszAddr, uint16_t ausPort);
bool CreateTcpServer(int aiSocket, const char* apszAddr, uint16_t ausPort,
                     bool abReuseAddr = true, bool abNoBlock = true, int aiRecvBuffSize = 0, int aiSendBuffSize = 0);
//create sync socket with recv timeout
int CreateSyncSocket(int aiRecvTimeout);

int GetNetError();
bool IsBlockingError(int aiError = 0);

//set max open file nums
void SetMaxOpenFiles(uint32_t auiMaxFileNums);
//set tcp connection keep living
int SetTcpSockKeepAlive(int nSocket, int nKeepAlive, int nKeepIdle, int nKeepInterval, int nKeepCnt);
//get ip by host
bool GetIpByHostName(const std::string& astrHost, std::string& astrIp);
//get peer addr
bool GetPeerAddr(int aiSocket, sockaddr_in& aoAddr);
#endif//_LIB_NET_H_
