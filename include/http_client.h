#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include "libnet.h"
#include "tcp_stream.h"
#include "host_ip_cache.h"

typedef std::map<std::string, std::string> URL_PARAMS;

class CHttpClient
{
public:

    enum ENUM_HTTP_ERROR {
        EHTTP_NO_ERROR,
        EHTTP_SUCCESS,
        EHTTP_URL_ERROR,
        EHTTP_GET_HOST_IP_FAIL,
        EHTTP_CRATE_SOCKET_FAIL,
        EHTTP_CONNECT_HOST_FAIL,
        EHTTP_CONNECT_TIMEOUT,
        EHTTP_SEND_DATA_FAIL,
        EHTTP_RECV_TIMEOUT,
        EHTTP_RECV_ERROR,
        EHTTP_NO_200_OK,
    };
    CHttpClient(void) {}
    ~CHttpClient(void) {}
    static bool GetHttpRequest(const std::string& astrUrl, std::string& astrRet, int liTimeout = 3000);
    static int GetHttpRequestEx(const std::string& astrUrl, std::string& astrRet, int liTimeout = 3000, bool abUseHostIpCache = false);
    static bool PostHttpRequest(const std::string& astrUrl, const std::string& astrPostContent, std::string& astrRet);
    static std::string UrlEncode(const std::string& astrData);
    static std::string UrlDecode(const std::string& astrData);
    static bool ParseParams(const std::string& astrParams, URL_PARAMS& amapParams,
                            const std::string& astrParamsSep = "&", const std::string& astrParamValueSep = "=");
    static bool SplitParams(const std::string& astrParams, std::vector<std::string>& avecParams, const std::string& astrSep = ",");
    static std::string GetParam(const URL_PARAMS& amapParams, const std::string& astrParamName);
    static bool GetHtmlData(const std::string& astrHtmlRes, std::string& astrHtmlData);
    static bool ParseUrl(const std::string& astrUrl, std::string& astrHost, std::string& astrPagePath, std::string& astrParams);
    static uint8_t CharToHex(const uint8_t& abyChValue);
    static uint8_t HexToChar(const uint8_t& abyChValue);
    static char Hex2Char(uint8_t abyChValue);
    static uint8_t Char2Hex(uint8_t abyChValue);
    static std::string Raw2HexStr(const char* apData, uint32_t auiDataLen);
    static std::string HexStr2Raw(const std::string& astrData);
    
    static bool ParseJsonParams(const std::string& astrParams, URL_PARAMS& amapParams);
public:
    static CHostIpCache moHostIpCache;
};
#endif //_HTTP_CLIENT_H_
