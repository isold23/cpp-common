#ifndef _BASE_64_H_
#define _BASE_64_H_

////////////////////////////////////////////////////////////////////////////////////
/**
* BASE64±àÂë£¬½âÂë
*/
#define BASE64_t2       0x30 //00110000
#define BASE64_m2       0x0c //00001100
#define BASE64_b2       0x03 //00000011
#define BASE64_t4       0x3c //00111100
#define BASE64_b4       0x0f //00001111
#define BASE64_h2       0xc0 //11000000
#define BASE64_b6       0x3f //00111111

static const char cBase64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static const char DeBase64Tab[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    62,        // '+'
    0, 0, 0,
    63,        // '/'
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,        // '0'-'9'
    0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,        // 'A'-'Z'
    0, 0, 0, 0, 0, 0,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,        // 'a'-'z'
};

class CBase64
{
public:
    CBase64(void) {}
    ~CBase64(void) {}
    static int Encode(char* DataOut, const char* DataIn, int size);
    static int Decode(char* DataOut, const char* DataIn, int size);
private:

};

inline int CBase64::Encode(char* DataOut, const char* DataIn, int size)
{
    int lOutputBufferLength;
    long n;
    long iGroupsOf3;
    long iLeftover;
    char* pBuffer;
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t r;
    // how many groups and leftovers
    iGroupsOf3 = size / 3;
    iLeftover = 3 - (size % 3);
    // every 3 characters become 4
    lOutputBufferLength = iGroupsOf3;
    
    if(iLeftover)
        lOutputBufferLength++;
        
    lOutputBufferLength *= 4;
    // create new buffer (with terminator byte)
    pBuffer = DataOut;
    
    // encode
    for(n = 0; n < (iGroupsOf3 * 3); n += 3) {
        a = DataIn[n + 0];        // aaaaaabb
        b = DataIn[n + 1];        // bbbbcccc
        c = DataIn[n + 2];        // ccdddddd
        // first character
        r = a;                    // aaaaaabb
        r = r >> 2;               // 00aaaaaa
        *pBuffer++ = cBase64Alphabet[r];
        // second character
        r = a;                    // aaaaaabb
        r = r & BASE64_b2;        // 000000bb
        r = r << 4;               // 00bb0000
        r = r + (b >> 4);         // 00bb0000 + 0000bbbb = 00bbbbbb
        *pBuffer++ = cBase64Alphabet[r];
        // third character
        r = b;                    // bbbbcccc
        r = b << 2;               // bbcccc00
        r = r & BASE64_t4;        // 00cccc00
        r = r + (c >> 6);         // 00cccc00 + 000000cc = 00cccccc
        *pBuffer++ = cBase64Alphabet[r];
        // fourth character
        r = c;                    // ccdddddd
        r = r & BASE64_b6;        // 00dddddd
        *pBuffer++ = cBase64Alphabet[r];
    }
    
    // handle non multiple of 3 data and insert padding
    if(iLeftover) {
        n = (iGroupsOf3 * 3);
        
        switch(iLeftover) {
        case 2:
            a = DataIn[n + 0];    // aaaaaabb
            b = 0;                //by lqh add ,......
            // first character
            r = a;                // aaaaaabb
            r = r >> 2;           // 00aaaaaa
            *pBuffer++ = cBase64Alphabet[r];
            // second character
            r = a;                // aaaaaabb
            r = r & BASE64_b2;    // 000000bb
            r = r << 4;           // 00bb0000
            r = r + (b >> 4);     // 00bbcccc
            *pBuffer++ = cBase64Alphabet[r];
            // insert padding x 2
            *pBuffer++ = cBase64Alphabet[64];
            *pBuffer++ = cBase64Alphabet[64];
            break;
            
        case 1:
            a = DataIn[n + 0];    // aaaaaabb
            b = DataIn[n + 1];    // bbbbcccc
            // first character
            r = a;                // aaaaaabb
            r = r >> 2;           // 00aaaaaa
            *pBuffer++ = cBase64Alphabet[r];
            // second character
            r = a;                // aaaaaabb
            r = r & BASE64_b2;    // 000000bb
            r = r << 4;           // 00bb0000
            r = r + (b >> 4);     // 00bbcccc
            *pBuffer++ = cBase64Alphabet[r];
            // third character
            r = b;                // bbbbcccc
            r = b << 2;           // bbcccc00
            r = r & BASE64_t4;    // 00cccc00
            *pBuffer++ = cBase64Alphabet[r];
            // insert padding
            *pBuffer++ = cBase64Alphabet[64];
            break;
        }
    }
    
    //test code
    int nTemp = (int)(pBuffer - DataOut);
    return nTemp;
}

inline int CBase64::Decode(char* pDst, const char* pSrc, int nSrcLen)
{
    int nDstLen;                  // .......
    int nValue;                   // ........
    int i;
    i = 0;
    nDstLen = 0;
    
    // .4....................3...
    while(i < nSrcLen) {
        if(*pSrc != '\r' && *pSrc != '\n') {
            nValue = DeBase64Tab[*pSrc++] << 18;
            nValue += DeBase64Tab[*pSrc++] << 12;
            *pDst++ = (nValue & 0x00ff0000) >> 16;
            nDstLen++;
            
            if(*pSrc != '=') {
                nValue += DeBase64Tab[*pSrc++] << 6;
                *pDst++ = (nValue & 0x0000ff00) >> 8;
                nDstLen++;
                
                if(*pSrc != '=') {
                    nValue += DeBase64Tab[*pSrc++];
                    *pDst++ = nValue & 0x000000ff;
                    nDstLen++;
                }
            }
            
            i += 4;
        } else {                  // .......
            pSrc++;
            i++;
        }
    }
    
    // .......
    *pDst = '\0';
    return nDstLen;
}

#endif  //_BASE_64_H_
