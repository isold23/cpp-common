#include "net_pack.h"
#include "base_encrypt.h"
#include "http_client.h"

class c_std_net_io : public i_net_io
{
public:
    virtual bool open()
    {
        return true;
    }
    virtual void close() {}
    virtual int  read(int ai_sock, void* ap_buf, int ai_num)
    {
        return recv(ai_sock, ap_buf, ai_num, 0);
    }
    virtual int  write(int ai_sock, const void* ap_buf, int ai_num)
    {
        return send(ai_sock, ap_buf, ai_num, 0);
    }
    
    virtual void release() {}
};

c_std_net_io go_std_net_io;

i_net_io* CNetPack::create_svr_io(int ai_sock)
{
    return &go_std_net_io;
}

i_net_io* CNetPack::create_clt_io(int ai_sock)
{
    return &go_std_net_io;
}


/************************************************************************/
/*
CNetPackVersion1
*/
/************************************************************************/
CNetPackVersion1::CNetPackVersion1()
{
    send_flag = DEF_NET_PACK_HEAD_PREFIX;
    send_version = 0;
    send_id = 0;
    send_time = 0;
    send_encry = 0;
    send_compress = 0;
    send_error_code = 0;
    send_length = 0;
    recv_flag = 0;
    recv_version = 0;
    recv_id = 0;
    recv_time = 0;
    recv_encry = 0;
    recv_compress = 0;
    recv_error_code = 0;
    recv_length = 0;
    _min_pack_size = 32;
    _max_pack_size = DEF_BUFFER_LEN;
}

int CNetPackVersion1::Pack(const char* in_buffer, const uint32_t in_length,
                           char* out_buffer, uint32_t& out_length)
{
    try {
        send_length = in_length;
        CStandardSerialize loSerialize(out_buffer, out_length, CStandardSerialize::STORE);
        memset(out_buffer, 0, out_length);
        
        if(Serialize(loSerialize) == -1)
            return -1;
        else {
            memcpy(out_buffer + _min_pack_size, in_buffer, in_length);
            out_length = _min_pack_size + in_length;
        }
        
        return 1;
    } catch(...) {
        return -1;
    }
}

int CNetPackVersion1::Unpack(const char* in_buffer, const uint32_t in_length,
                             char* out_buffer, uint32_t& out_buffer_length, uint32_t& out_data_length)
{
    try {
        CStandardSerialize loSerialize((char*)in_buffer, in_length, CStandardSerialize::LOAD);
        
        if(Serialize(loSerialize) == -1) {
            return -1;
        } else {
            if(CheckPack()) {
                if(in_length >= _min_pack_size + recv_length) {
                    memcpy(out_buffer, in_buffer + _min_pack_size, recv_length);
                    out_buffer_length = recv_length;
                    out_data_length = recv_length + _min_pack_size;
                } else {
                    return 0;
                }
            } else {
                return -1;
            }
        }
        
        return 1;
    } catch(...) {
        return -1;
    }
}

int CNetPackVersion1::Serialize(CStandardSerialize& aoStandardSerialize)
{
    try {
        if(aoStandardSerialize.mbyType == CStandardSerialize::STORE) {
            aoStandardSerialize.Serialize(send_flag);
            aoStandardSerialize.Serialize(send_version);
            aoStandardSerialize.Serialize(send_id);
            aoStandardSerialize.Serialize(send_time);
            aoStandardSerialize.Serialize(send_encry);
            aoStandardSerialize.Serialize(send_compress);
            aoStandardSerialize.Serialize(send_error_code);
            aoStandardSerialize.Serialize(send_length);
        } else if(aoStandardSerialize.mbyType == CStandardSerialize::LOAD) {
            aoStandardSerialize.Serialize(recv_flag);
            aoStandardSerialize.Serialize(recv_version);
            aoStandardSerialize.Serialize(recv_id);
            aoStandardSerialize.Serialize(recv_time);
            aoStandardSerialize.Serialize(recv_encry);
            aoStandardSerialize.Serialize(recv_compress);
            aoStandardSerialize.Serialize(recv_error_code);
            aoStandardSerialize.Serialize(recv_length);
        }
        
        return 1;
    } catch(...) {
        return -1;
    }
}

/************************************************************************/
/*
CNetPackVersion2
*/
/************************************************************************/
CNetPackVersion2::CNetPackVersion2()
{
    //发送数据头
    send_flag = 0;
    send_version = 0;
    send_length = 0;
    send_encry = 0;
    send_extend1 = 0;
    send_extend2 = 0;
    send_check_key = 0;
    send_id = 0;
    //接收数据头
    recv_flag = 0;
    recv_version = 0;
    recv_length = 0;
    recv_encry = 0;
    recv_extend1 = 0;
    recv_extend2 = 0;
    recv_check_key = 0;
    recv_id = 0;
    _min_pack_size = 24;
    _max_pack_size = DEF_BUFFER_LEN;
}

CNetPackVersion2::~CNetPackVersion2()
{
}

int CNetPackVersion2::Pack(const char* in_buffer, const uint32_t in_length,
                           char* out_buffer, uint32_t& out_length)
{
    try {
        std::string strEncrypt = "";
        
        if(send_encry == 2) {
            if(send_id == 0) {
                CNetPackVersion1* p = NULL;
                p->_max_pack_size = 0;
                TRACE(1, "CNetPackVersion2::Pack send id = " << send_id);
                return -1;
            }
            
            std::string strBuffer(in_buffer, in_length);
            std::string strKey = GetEncryKey(send_id, 2);
            CAESEncrypt aes_encrypt;
            bool nRet = aes_encrypt.Encrypt(strBuffer, strKey, strEncrypt);
            
            if(!nRet) {
                TRACE(1, "CNetPackVersion2::Pack 加密失败。");
                return -1;
            }
        } else {
            strEncrypt = std::string(in_buffer, in_length);
            ASSERT(strEncrypt.size() == in_length);
        }
        
        send_length = strEncrypt.size();
        CStandardSerialize loSerialize(out_buffer, out_length, CStandardSerialize::STORE);
        memset(out_buffer, 0, out_length);
        
        if(Serialize(loSerialize) == -1)
            return -1;
        else {
            memcpy(out_buffer + _min_pack_size, strEncrypt.c_str(), strEncrypt.size());
            out_length = _min_pack_size + strEncrypt.size();
        }
        
        return 1;
    } catch(...) {
        return -1;
    }
}
int CNetPackVersion2::Unpack(const char* in_buffer, const uint32_t in_length,
                             char* out_buffer, uint32_t& out_buffer_length, uint& out_data_length)
{
    try {
        CStandardSerialize loSerialize((char*)in_buffer, in_length, CStandardSerialize::LOAD);
        
        if(Serialize(loSerialize) == -1) {
            return -1;
        } else {
            if(CheckPack()) {
                if(in_length >= _min_pack_size + recv_length) {
                    std::string strEncrypt = "";
                    
                    //判断是否需要解密
                    if(recv_encry == 2) {
                        std::string strBuffer(in_buffer + _min_pack_size, recv_length);
                        std::string strKey = GetEncryKey(recv_id, recv_encry);
                        CAESEncrypt aes_encrypt;
                        bool nRet = aes_encrypt.Decrypt(strBuffer, strKey, strEncrypt);
                        
                        if(!nRet) {
                            TRACE(1, "CNetPackVersion2::Unpack 解密失败。");
                            return -1;
                        }
                    } else {
                        strEncrypt = std::string((in_buffer + _min_pack_size), recv_length);
                    }
                    
                    memcpy(out_buffer, strEncrypt.c_str(), strEncrypt.size());
                    out_buffer_length = strEncrypt.size();
                    out_data_length = recv_length + _min_pack_size;
                } else {
                    return 0;
                }
            } else {
                return -1;
            }
        }
        
        return 1;
    } catch(...) {
        TRACE(1, "CNetPackVersion2::Unpack 出现异常");
        return -1;
    }
}
int CNetPackVersion2::Serialize(CStandardSerialize& aoStandardSerialize)
{
    try {
        if(aoStandardSerialize.mbyType ==  CStandardSerialize::STORE) {
            aoStandardSerialize.Serialize(send_flag);
            aoStandardSerialize.Serialize(send_version);
            aoStandardSerialize.Serialize(send_length);
            aoStandardSerialize.Serialize(send_encry);
            aoStandardSerialize.Serialize(send_extend1);
            aoStandardSerialize.Serialize(send_extend2);
            aoStandardSerialize.Serialize(send_check_key);
            aoStandardSerialize.Serialize(send_id);
        } else if(aoStandardSerialize.mbyType == CStandardSerialize::LOAD) {
            aoStandardSerialize.Serialize(recv_flag);
            aoStandardSerialize.Serialize(recv_version);
            aoStandardSerialize.Serialize(recv_length);
            aoStandardSerialize.Serialize(recv_encry);
            aoStandardSerialize.Serialize(recv_extend1);
            aoStandardSerialize.Serialize(recv_extend2);
            aoStandardSerialize.Serialize(recv_check_key);
            aoStandardSerialize.Serialize(recv_id);
        }
        
        return 1;
    } catch(...) {
        return -1;
    }
}

/************************************************************************/
/*
CNetPackVersion3
*/
/************************************************************************/
int CNetPackVersion3::Pack(const char* in_buffer, const uint32_t in_length,
                           char* out_buffer, uint32_t& out_length)
{
    try {
        CStandardSerialize loSerialize(out_buffer, out_length, CStandardSerialize::STORE);
        memset(out_buffer, 0, out_length);
        
        if(Serialize(loSerialize) == -1)
            return -1;
        else {
            memcpy(out_buffer + _min_pack_size, in_buffer, in_length);
            out_length = _min_pack_size + in_length;
        }
        
        return 1;
    } catch(...) {
        return -1;
    }
}
int CNetPackVersion3::Unpack(const char* in_buffer, const uint32_t in_length,
                             char* out_buffer, uint32_t& out_buffer_length, uint32_t& out_data_length)
{
    try {
        CStandardSerialize loSerialize((char*)in_buffer, in_length, CStandardSerialize::LOAD);
        
        if(Serialize(loSerialize) == -1) {
            return -1;
        } else {
            if(CheckPack()) {
                if(in_length >= _min_pack_size + recv_pack_length) {
                    memcpy(out_buffer, in_buffer + _min_pack_size, recv_pack_length);
                    out_buffer_length = recv_pack_length;
                    out_data_length = recv_pack_length + _min_pack_size;
                } else {
                    return 0;
                }
            } else {
                return -1;
            }
        }
        
        return 1;
    } catch(...) {
        TRACE(1, "version3 unpacke exception.");
        return -1;
    }
}
int CNetPackVersion3::Serialize(CStandardSerialize& aoStandardSerialize)
{
    try {
        if(aoStandardSerialize.mbyType ==  CStandardSerialize::STORE) {
            aoStandardSerialize.Serialize(send_pack_length);
        } else if(aoStandardSerialize.mbyType == CStandardSerialize::LOAD) {
            aoStandardSerialize.Serialize(recv_pack_length);
            recv_pack_length = ntohl(recv_pack_length);
        }
        
        return 1;
    } catch(...) {
        TRACE(1, "version3 serialize exception.");
        return -1;
    }
}
