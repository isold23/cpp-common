#ifndef _LMENDECRYPT_H_
#define _LMENDECRYPT_H_

#include <stdint.h>
#define N 16

class LMEnDecrypt
{
public:
    LMEnDecrypt();
    LMEnDecrypt(char* apKey, int aiNum);
    ~LMEnDecrypt();
    void InitPassword(const char* apKey, int aiNum);
    int LmEncrypt(char* apInData, uint16_t aiInDataLen, char* apOutData, uint16_t& aiOutDataLen);
    int LmDecrypt(char* apInData, uint16_t aiInDataLen, char* apOutData, uint16_t& aiOutDataLen);
    
private:
    uint32_t F(uint32_t x);
    void Blowfish_encipher(uint32_t* xl, uint32_t* xr);
    void Blowfish_decipher(uint32_t* xl, uint32_t* xr);
    
private:
    uint32_t P[N + 2];
    uint32_t S[4][256];
};
#endif //_LMENDECRYPT_H_
