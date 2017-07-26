#include "udp_socket.h"

CUdpSocket::CUdpSocket()
{
    miSocket = -1;
}

CUdpSocket::~CUdpSocket()
{
    miSocket = -1;
    Close();
}

bool CUdpSocket::CreateSocket(void)
{
    miSocket = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(miSocket < 0) {
        TRACE(1, "CUdpSocket::CreateSocket socket() 失败。errno = " << errno);
        return false;
    }
    
    return true;
}

bool CUdpSocket::CreateSocket(const char* ip, const short port)
{
    miSocket = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(miSocket < 0) {
        TRACE(1, "CUdpSocket::CreateSocket socket() 失败。errno = " << errno);
        return false;
    }
    
    //int iReuseAddr = 1;
    //setsockopt(miSocket, SOL_SOCKET, SO_REUSEADDR, (void*)(&(iReuseAddr))
    //  , sizeof(iReuseAddr));
    struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    
    if(ip) {
        stAddr.sin_addr.s_addr = inet_addr(ip);
    } else {
        stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    stAddr.sin_port = htons(port);
    socklen_t loinlen = sizeof(sockaddr_in);
    
    if(bind(miSocket, (struct sockaddr*)&stAddr, loinlen) < 0) {
        TRACE(1, "CUdpSocket::CreateSocket bind() 失败。port = " << port <<
              " ip = " << ip << " errno = " << errno);
        close(miSocket);
        return false;
    }
    
    TRACE(1, "CUdpSocket::CreateSocket bind() 成功。port = " << port <<
          " ip = " << ip);
    return true;
}

bool CUdpSocket::Close(int flag /* = 2 */)
{
    shutdown(miSocket, flag);
    close(miSocket);
    return true;
}

bool CUdpSocket::SetNoBlock()
{
    int iFlags = 1; // nonblock reusaddr
    ioctl(miSocket, FIONBIO, &iFlags);
    iFlags = fcntl(miSocket, F_GETFL, 0);
    int nRet = fcntl(miSocket, F_SETFL, iFlags | O_NONBLOCK | O_NDELAY);
    
    if(nRet == -1) {
        TRACE(1, "CNetSocket::SetNoBlock 设置非阻塞属性失败。socket : " << miSocket);
        return false;
    }
    
    return true;
}

/************************************************************************
函数功能：
    根据指定地址发送数据包，在数据包前加上通用协议头。
参数说明：
    buffer:发送数据包缓冲区。
    length:发送数据包长度。
    dst_ip:数据包目标IP地址。
    dst_port:数据包目标端口。
************************************************************************/
bool CUdpSocket::SendData(const char* buffer, const uint32_t length,
                          uint32_t dst_ip, const uint16_t dst_port)
{
    sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sockaddr_in));
    CCommon::SetSocketAddr(sock_addr, htonl(dst_ip), htons(dst_port));
    uint32_t nSend  = 0;
    char send_buffer[DEF_UDP_PACK_BUF_LEN] = {0};
    uint32_t send_len = DEF_UDP_PACK_BUF_LEN;
    CNetPackVersion1 pack;
    pack.Pack(buffer, length, send_buffer, send_len);
    /*  CNetPackHead header;
        header.miLength = length;
        int nRet = header.Pack(send_buffer, send_len);
        if(nRet <= 0)
        {
            return false;
        }
        memcpy(send_buffer + nRet, buffer, length);
        send_len = nRet + length;
        */
    nSend  = sendto(miSocket, send_buffer, send_len, 0, (sockaddr*) &sock_addr, sizeof(sockaddr_in));
    return true;
}

/************************************************************************
函数功能：
    接收数据包，对数据包去除通用协议包头后，将数据包返给上层应用。
参数说明：
    buffer: 接收收到数据的缓冲区。
    length:指定buffer缓冲区的大小，返回后保存实际接收数据包大小。
    src_ip： 数据包来源IP地址,本地字节序。
    src_port:数据包来源端口，本地字节序。
************************************************************************/
bool CUdpSocket::RecvData(char* buffer, uint32_t& length,
                          uint32_t& src_ip, uint16_t& src_port)
{
    //uint32_t auiRecvLen = 0;
    sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sockaddr_in));
    //uint32_t sock_addr_size = 0;
    uint32_t sockaddr_len = sizeof(sockaddr_in);
    char recv_buffer[DEF_UDP_PACK_BUF_LEN] = {0};
    int32_t recv_len = DEF_UDP_PACK_BUF_LEN;
    recv_len = recvfrom(miSocket, recv_buffer, DEF_UDP_PACK_BUF_LEN, 0,
                        (sockaddr*)&sock_addr, &sockaddr_len);
                        
    if(recv_len <= 0) {
        return false;
    } else if(recv_len < 32) {
        TRACE(1, "CUdpSocket::RecvData 收到数据包长度小于最小包长度。recv len : " << recv_len);
        return false;
    } else {
        uint32_t data_len = 0;
        uint32_t buffer_len = 0;
        CNetPackVersion1 pack;
        pack.Unpack((const char*)recv_buffer, recv_len, buffer, buffer_len, data_len);
        length = buffer_len;
        CCommon::GetSocketAddr(sock_addr, src_ip, src_port);
        /*  CNetPackHead header;
            int nRet = header.Unpack(recv_buffer, recv_len);
            if(nRet > 0)
            {
                bool bRet = header.CheckHead();
                if(bRet)
                {
                    memcpy(buffer, recv_buffer  + sizeof(CNetPackHead), header.miLength);
                    length = recv_len;
                    CCommon::GetSocketAddr(sock_addr, src_ip, src_port);
                    return true;
                }
                else
                {
                    TRACE(1, "CUdpSocket::RecvData  检测头失败。");
                    return false;
                }
            }
            else
            {
                TRACE(1, "CUdpSocket::RecvData  解包失败。");
                return false;
            }*/
    }
    
    return true;
}

/************************************************************************
函数功能：
    根据指定地址发送数据包，在数据包前不加通用协议头。
参数说明：
    buffer:发送数据包缓冲区。
    length:发送数据包长度。
    dst_ip:数据包目标IP地址。
    dst_port:数据包目标端口。
************************************************************************/
bool CUdpSocket::SendDataWithoutHeader(const char* buffer, const uint32_t length,
                                       const uint32_t dst_ip, const uint16_t dst_port)
{
    sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sockaddr_in));
    CCommon::SetSocketAddr(sock_addr, htonl(dst_ip), htons(dst_port));
    int32_t nSend  = sendto(miSocket, buffer, length, 0, (sockaddr*) &sock_addr, sizeof(sockaddr_in));
    
    if(nSend < 0) {
        TRACE(1, "send data failed. errno: " << errno);
        return false;
    }
    
    return true;
}

/************************************************************************
函数功能：
    接收数据包，对数据包不进行任何处理，直接将数据包返给上层应用。
参数说明：
    buffer: 接收收到数据的缓冲区。
    length:指定buffer缓冲区的大小，返回后保存实际接收数据包大小。
    src_ip： 数据包来源IP地址，本地字节序。
    src_port:数据包来源端口，本地字节序。
************************************************************************/
bool CUdpSocket::RecvDataWithoutHeader(char* buffer, uint32_t& length,
                                       uint32_t& src_ip, uint16_t& src_port)
{
    //uint32_t auiRecvLen = 0;
    sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sockaddr_in));
//  uint32_t sock_addr_size = 0;
    uint32_t sockaddr_len = sizeof(sockaddr_in);
    char recv_buffer[DEF_UDP_PACK_BUF_LEN] = {0};
    uint32_t recv_len = DEF_UDP_PACK_BUF_LEN;
    recv_len = recvfrom(miSocket, recv_buffer, DEF_UDP_PACK_BUF_LEN, 0,
                        (sockaddr*)&sock_addr, &sockaddr_len);
                        
    if(recv_len <= 0) {
        return false;
    } else {
        ASSERT(recv_len <= length);
        CCommon::GetSocketAddr(sock_addr, src_ip, src_port);
        memcpy(buffer, recv_buffer, recv_len);
        length = recv_len;
    }
    
    return true;
}

void CUdpSocket::Dump()
{
}

void CUdpSocket::TimeWorkFunction()
{
}











