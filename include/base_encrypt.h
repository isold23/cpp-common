#ifndef _BASE_ENCRYPT_H_
#define _BASE_ENCRYPT_H_

#include <openssl/aes.h>
#include "include.h"

class CBaseEncrypt
{
public:
    CBaseEncrypt() {}
    virtual ~CBaseEncrypt() {}
    
    virtual bool Encrypt(const std::string& astrData, const std::string& astrKey, std::string& astrResult) = 0;
    virtual bool Decrypt(const std::string& astrData, const std::string& astrKey, std::string& astrResult) = 0;
};

class CAESEncrypt : public CBaseEncrypt
{
public:
    CAESEncrypt() {}
    virtual ~CAESEncrypt() {}
    
    bool Encrypt(const std::string& astrData, const std::string& astrKey, std::string& astrResult)
    {
        if(astrKey.size() != AES_BLOCK_SIZE) return false;
        
        AES_KEY key;
        AES_set_encrypt_key((const uint8_t*)astrKey.c_str(), 128, &key);
        int liLen = astrData.size();
        int liOutLen = liLen;
        
        if((liLen % AES_BLOCK_SIZE) != 0) {
            liOutLen = ((liLen / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
        }
        
        uint8_t* lpOutBuff = new uint8_t[liOutLen];
        uint8_t iv[16] = {0};
        AES_cbc_encrypt((const uint8_t*)astrData.c_str(), lpOutBuff, liOutLen, &key, iv, AES_ENCRYPT);
        astrResult = std::string((char*)lpOutBuff, liOutLen);
        delete[] lpOutBuff;
        lpOutBuff = NULL;
        return true;
    }
    
    bool Decrypt(const std::string& astrData, const std::string& astrKey, std::string& astrResult)
    {
        if(astrKey.size() != AES_BLOCK_SIZE) return false;
        
        AES_KEY key;
        AES_set_decrypt_key((const uint8_t*)astrKey.c_str(), 128, &key);
        int liLen = astrData.size();
        int liOutLen = liLen;
        
        if((liLen % AES_BLOCK_SIZE) != 0) {
            liOutLen = ((liLen / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
        }
        
        uint8_t* lpOutBuff = new uint8_t[liOutLen + 1];
        memset(lpOutBuff, 0, liOutLen + 1);
        uint8_t iv[16] = {0};
        AES_cbc_encrypt((const uint8_t*)astrData.c_str(), lpOutBuff, liOutLen, &key, iv, AES_DECRYPT);
        astrResult = std::string((char*)lpOutBuff, liOutLen);
        delete[] lpOutBuff;
        lpOutBuff = NULL;
        return true;
    }
};
#endif //_BASE_ENCRYPT_H_
