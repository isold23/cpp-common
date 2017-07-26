/********************************************************************
author:     isold.wang@gmail.com
*********************************************************************/
#ifndef _NET_PACK_H_
#define _NET_PACK_H_

#include "include.h"
#include "standard_serialize.h"

#define DEF_NET_PACK_HEAD_PREFIX 0x99
#define DEF_BUFFER_LEN (1024*1024)

class i_net_io
{
public:
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual int  read(int ai_sock, void* ap_buf, int ai_num) = 0;
    virtual int  write(int ai_sock, const void* ap_buf, int ai_num) = 0;
    
    virtual void release() = 0;
};

class CNetPack
{
public:
    CNetPack() {}
    virtual ~CNetPack() {}
    virtual bool CheckPack() = 0;
    virtual int Pack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_length) = 0;
    virtual int Unpack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_buffer_length, uint32_t& out_data_length) = 0;
    
public:
    virtual i_net_io* create_svr_io(int ai_sock);
    virtual i_net_io* create_clt_io(int ai_sock);
    
public:
    uint32_t _min_pack_size;
    uint32_t _max_pack_size;
};

class CNetPackVersion1 : public CNetPack
{
public:
    CNetPackVersion1();
    virtual ~CNetPackVersion1() {}
    
    int Pack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_length);
    int Unpack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_buffer_length, uint32_t& out_data_length);
    
    bool CheckPack()
    {
        if((recv_flag == send_flag) &&
                (recv_length <= (_max_pack_size - _min_pack_size)) &&
                (recv_length >= 2)) {
            return true;
        } else {
            TRACE(1, "CNetPackHead::CheckHead check error. recv flag: " <<
                  recv_flag << " Length: " << recv_length);
            return false;
        }
    }
private:
    int Serialize(CStandardSerialize& aoStandardSerialize);
    
public:
    //send
    int send_flag;
    int send_version;
    int send_id;
    int send_time;
    int send_encry;
    int send_compress;
    int send_error_code;
    int send_length;
    
    //recv
    int recv_flag;
    int recv_version;
    int recv_id;
    int recv_time;
    int recv_encry;
    int recv_compress;
    int recv_error_code;
    uint32_t recv_length;
};

class CNetPackVersion2 : public CNetPack
{
public:
    CNetPackVersion2();
    virtual ~CNetPackVersion2();
    
    int Pack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_length);
    int Unpack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_buffer_length, uint32_t& out_data_length);
    
    bool CheckPack()
    {
        if((recv_flag == send_flag) &&
                (recv_length <= (_max_pack_size - _min_pack_size)) &&
                (recv_length >= 2)) {
            return true;
        } else {
            TRACE(1, "CNetPackHead::CheckHead 协议检测失败。FLAG = " <<
                  recv_flag << " Length = " << recv_length);
            return false;
        }
    }
    
private:
    int Serialize(CStandardSerialize& aoStandardSerialize);
    std::string GetEncryKey(uint64_t user_id, uint32_t encry_type)
    {
        stringstream  strStream;
        strStream << user_id;
        std::string strUserId = "";
        strStream >> strUserId;
        char strMd5[33];
        memset(strMd5, 0, 33);
        CCommon::MakeMD5WithBuffer32((uint8_t*)strUserId.c_str(), strUserId.length(), (uint8_t*)strMd5);
        char strEncryKey[17];
        memset(strEncryKey, 0, 17);
        memcpy(strEncryKey, strMd5 + 7, 16);
        return strEncryKey;
    }
    
public:
    //发送数据头
    int32_t send_flag;
    int16_t send_version;
    int16_t send_length;
    int8_t send_encry;
    int8_t send_extend1;
    int16_t send_extend2;
    int32_t send_check_key;
    int64_t send_id;
    
    //接收数据头
    int32_t recv_flag;
    int16_t recv_version;
    uint16_t recv_length;
    int8_t recv_encry;
    int8_t recv_extend1;
    int16_t recv_extend2;
    int32_t recv_check_key;
    int64_t recv_id;
};

class CNetPackVersion3 : public CNetPack
{
public:
    CNetPackVersion3()
    {
        _min_pack_size = 4;
        _max_pack_size = DEF_BUFFER_LEN;
    }
    virtual ~CNetPackVersion3() {}
    
    int Pack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_length);
    int Unpack(const char* in_buffer, const uint32_t in_length, char* out_buffer, uint32_t& out_buffer_length, uint32_t& out_data_length);
    
    bool CheckPack()
    {
        if((recv_pack_length <= (_max_pack_size - _min_pack_size)) &&
                (recv_pack_length >= 2)) {
            return true;
        } else {
            TRACE(1, "check pack length = " << recv_pack_length);
            return false;
        }
        
        return true;
    }
    
private:
    int Serialize(CStandardSerialize& aoStandardSerialize);
    
public:
    uint32_t send_pack_length;
    uint32_t recv_pack_length;
};
#endif //_NET_PACK_H_











