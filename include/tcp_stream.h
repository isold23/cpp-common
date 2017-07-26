#ifndef _TCP_STREAM_H_
#define _TCP_STREAM_H_

#include <stdint.h>

class CTcpStream
{
public:
    CTcpStream(int liTimeout = 1000);
    ~CTcpStream(void);
    bool Connect(const char* apszDestIp, uint16_t ausPort, int liTimeout = 0);
    bool SendData(const char* apData, uint16_t ausDataLen);
    int RecvData(char* apBuff, uint16_t ausBuffSize, int liTimeout = 1000);
    void Disconnect();
    int GetSocket()
    {
        return miSocket;
    }
private:
    int miSocket;
};

#endif //_TCP_STREAM_H_

