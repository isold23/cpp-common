#ifndef _CONVERT_CHARSET_H_
#define _CONVERT_CHARSET_H_

#include "include.h"

class CConverter
{
public:
    CConverter(const char* pFrom_code_name, const char* pTo_code_name)
    {
        m_conv_fd = iconv_open(pTo_code_name, pFrom_code_name);
    }
    CConverter() : m_conv_fd(reinterpret_cast<iconv_t>(static_cast<intptr_t>(-1))) {};
    ~CConverter() {}
    int Convert(char** ppTarget, size_t* pTarget_len, char** ppSrc, size_t* pSrc_len)
    {
        assert(ppTarget && *ppTarget && pTarget_len && ppSrc && *ppSrc && pSrc_len);
        size_t uTargetMaxLen = *pTarget_len;
        LOG_DEBUG("src len : %zu", *pSrc_len);
        size_t uSrcLen = *pSrc_len;
        LOG_DEBUG("src len : %zu usrclen: %zu", *pSrc_len, uSrcLen);
        
        if(m_conv_fd !=  reinterpret_cast<iconv_t>(static_cast<intptr_t>(-1))) {
            int count = 0;
            size_t error_index = 0;
            while( *pSrc_len > 0 && *pTarget_len > 0) {
                int nResult = static_cast<int>(iconv(m_conv_fd, ppSrc, pSrc_len, ppTarget, pTarget_len));
                int error_num = errno;

                if(nResult > -1) {
                    *pTarget_len = uTargetMaxLen - *pTarget_len;
                    return nResult;
                } else if(nResult == -1) {
                    ++count;
                    if(error_num != E2BIG) {
                        if(error_index - 1 != *pSrc_len) { 
                            **ppTarget='?';
                            *ppTarget = *ppTarget + 1;
                            *pTarget_len = *pTarget_len - 1;
                        }
                        error_index = *pSrc_len;
                        *ppSrc = *ppSrc + 1;
                        *pSrc_len = *pSrc_len - 1;
                    } else {
                        *pTarget_len = uTargetMaxLen - *pTarget_len;
                        return 0;
                    }
                    
                    if(count == 1) {
                        LOG_NOTICE("convert charset failed. errno: %d src len: %d, psrc_len: %d ptarget_len: %d", errno, uSrcLen, *pSrc_len, *pTarget_len);
                    }
                }
            }
            *pTarget_len = uTargetMaxLen - *pTarget_len;
            return 0;
        }
        if(*pTarget_len > *pSrc_len) {
            *pTarget_len = *pSrc_len;
        }
        memcpy(*ppTarget, *ppSrc, *pTarget_len);
        return static_cast<int>(*pTarget_len);
    }
private:
    iconv_t m_conv_fd;
};

inline int u2g(char* pOut, size_t& uOutLen, const size_t uMaxOutLen, char* pIn, size_t uInLen)
{
    //START_COUNT_TIME(u2g);
    assert(pOut && pIn);
    static CConverter CodeConvert("UTF-8//TRANSLIT", "GBK");
    //static CConverter CodeConvert("UTF-8//IGNORE", "GBK");
    char** ppDest = &pOut;
    char** ppSrc = &pIn;
    uOutLen = uMaxOutLen;
    size_t inlen = uInLen;
    LOG_DEBUG("in len : %d", uInLen);
    CodeConvert.Convert(ppDest, &uOutLen, ppSrc, &inlen);
    //pOut[uOutLen] = '\0';
    //END_COUNT_TIME(u2g);
    return uOutLen;
}
#endif //_CONVERT_CHARSET_H_
