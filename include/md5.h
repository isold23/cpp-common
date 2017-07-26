#ifndef _MD5_H_
#define _MD5_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

class MD5_CTX
{
public:
    MD5_CTX();
    ~MD5_CTX();
    
    void MD5Update(uint8_t* input, uint32_t inputLen);
    void MD5Final(uint8_t digest[16]);
    
private:
    uint32_t state[4];      /* state (ABCD) */
    uint32_t count[2];      /* number of bits, modulo 2^64 (lsb first) */
    uint8_t buffer[64];       /* input buffer */
    uint8_t PADDING[64];      /* What? */
    
private:
    void MD5Init();
    void MD5Transform(uint32_t state[4], uint8_t block[64]);
    void MD5_memcpy(uint8_t* output, uint8_t* input, uint32_t len);
    void Encode(uint8_t* output, uint32_t* input, uint32_t len);
    void Decode(uint32_t* output, uint8_t* input, uint32_t len);
    void MD5_memset(uint8_t* output, int value, uint32_t len);
};

#endif //_MD5_H_






