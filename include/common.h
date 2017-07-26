/********************************************************************
    created:    2011/03/08
    created:    8:3:2011   10:39
    file base:  Common
    file ext:   h
    author:     isold.wang@gmail.com
*********************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <grp.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <iconv.h>
#include <string.h>
#include <sys/timeb.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

using std::cout;
using std::setw;
using std::ends;

#include "define.h"
#include "debugtrace.h"
#include "md5.h"
struct Passwd {
    static const uint8_t  BUFF_LEN    =   200;
    typedef struct passwd passwd_t;
    passwd_t    moPd;
    passwd_t*   motmpPwd;
    char        mcbuffer[ BUFF_LEN ];
    Passwd()
    {
        memset(this, 0, sizeof(Passwd));
    }
    ~Passwd()
    {
        memset(this, 0, sizeof(Passwd));
    }
};

template <int v>
struct Int2Type {
    enum { value = v };
};

template <short v>
struct Short2Type {
    enum { value = v };
};

class CCommon
{
public:
    static uint32_t GetProcessId();
    static uint32_t GetProcessParentId();
    static void SetOpenFiles(uint32_t max_file_opens_number);
    static void GetAppPath(char* apPath, int aiLen);
    static void CreatePath(char* apPath);
    static uint64_t get_current_time(void);
    static uint64_t escape_time(uint64_t last_time);
    
    //获取当前时间 2009-12-02 12:02:30:234
    static std::string get_now(void);
    //检测是否超时
    static bool CheckTimeOut(const int64_t& ai64LastTime, const int64_t& ai64TimeInterval);
    
    //根据IP地址和包序号计算主键
    static int64_t MakeIpPortKey(const sockaddr_in& addr, const uint16_t awPackSerial);
    static uint32_t GetSocketAddr(const sockaddr_in& addr);
    static uint32_t GetSocketAddr(const char* addr);
    static void SetSocketAddr(sockaddr_in& addr, uint32_t alAddr);
    static void SetSocketAddr(sockaddr_in& addr, uint32_t alAddr, uint16_t alPort);
    static char* GetIPAddr(uint32_t alAddr);
    static char* Getctime(time_t* apTime);
    
    //获取本地字节序地址
    static void GetSocketAddr(const sockaddr_in& addr, uint32_t& ip, uint16_t& port);
    //封装输出调用堆栈的功能
    static void DumpStack();
    
    template <class T>
    static void PeriodicTask(uint32_t time_gap, uint64_t& last_time, T* ptr, void (T::*fun)())
    {
        uint64_t time_now = CCommon::get_current_time();
        
        if(time_now > last_time + time_gap) {
            (ptr->*fun)();
        }
        
        last_time = time_now;
    }
    //取得一个随机整数,取值范围在 1 -- rand_max,rand_max必须大于1
    static uint32_t GetRandInt(uint32_t rand_max);
    
    //将字符串的内容随机变化顺序,aiChgTimes指定变换次数
    static std::string ChgStrInRandomSeq(const std::string& astrData, int aiChgTimes = 3);
    
    // 功   能：根据用户账户Name获得用户信息。
    // 参   数：lszUserName 用户账户名，struct Passwd* 一个预先实例化的同类型对象。
    // 返   回：成功 SUCCEEDED， 失败 FAILED。
    static uint8_t GetPasswd(const char* lszUserName, struct Passwd* opPasswd);
    
    // 功   能：根据用户账户Name设置子进程运行时的用户和用户组权限。
    // 参   数：lszUserName 将要指定的用户账户Name。
    // 返   回：无
    static void SetUA2SomeBody(const char* lszUserName);
    
    // 功   能：获得当前的工作路径。
    // 参   数：oszcPath 预先分配的用于存储路径的内存空间， ilen 内存空间的大小。
    // 返   回：成功 SUCCEEDED， 失败 FAILED。
    static uint8_t GetCurWorkingPath(char* oszcPath, int ilen);
    
    //获取md5值 16字节
    static int MakeMD5WithBuffer16(uint8_t* Input, uint InputLen, uint8_t* Output)
    {
        MD5_CTX md5;
        md5.MD5Update(Input, InputLen);
        md5.MD5Final(Output);
        return 0;
    }
    
    //数值转成字符串
    static void ConvertString(char* apIn, char* alpOut)
    {
        uint8_t lcTemp = 0;
        
        for(int i = 0; i < 16; i++) {
            lcTemp = apIn[i];
            lcTemp = lcTemp & 0xF0;
            lcTemp = lcTemp >> 4;
            
            if(lcTemp < 10)
                alpOut[2 * i] = lcTemp + '0';
            else
                alpOut[2 * i] = lcTemp + 87;
                
            lcTemp = apIn[i];
            lcTemp = lcTemp & 0x0F;
            
            if(lcTemp < 10)
                alpOut[2 * i + 1] = lcTemp + '0';
            else
                alpOut[2 * i + 1] = lcTemp + 87;
        }
        
        alpOut[32] = 0;
    }
    
    //获取md5值 32位
    static int MakeMD5WithBuffer32(uint8_t* Input, uint InputLen, uint8_t* Output)
    {
        uint8_t lpMD5[16];
        MD5_CTX md5;
        md5.MD5Update(Input, InputLen);
        md5.MD5Final(lpMD5);
        CCommon::ConvertString((char*)lpMD5, (char*)Output);
        return 0;
    }
    
    static int compute_file_md5(const char* file_path, char* md5_str)
    {
#define READ_DATA_SIZE  1024
#define MD5_SIZE        16
#define MD5_STR_LEN     (MD5_SIZE * 2)
        int i;
        int fd;
        int ret;
        unsigned char data[READ_DATA_SIZE];
        unsigned char md5_value[MD5_SIZE];
        MD5_CTX md5;
        fd = open(file_path, O_RDONLY);
        
        if(-1 == fd) {
            perror("open");
            return -1;
        }
        
        while(1) {
            ret = read(fd, data, READ_DATA_SIZE);
            
            if(-1 == ret) {
                perror("read");
                return -1;
            }
            
            md5.MD5Update(data, ret);
            
            if(0 == ret || ret < READ_DATA_SIZE) {
                break;
            }
        }
        
        close(fd);
        md5.MD5Final(md5_value);
        
        for(i = 0; i < MD5_SIZE; i++) {
            snprintf(md5_str + i * 2, 2 + 1, "%02x", md5_value[i]);
        }
        
        md5_str[MD5_STR_LEN] = '\0';
        return 0;
    }
};

template <class State, class T>
class AbstractFSM
{
public:
    AbstractFSM() {}
    ~AbstractFSM() {}
    
    void SetState(State st)
    {
        _st = st;
    }
    
    State GetState(void)
    {
        return _st;
    }
    
    bool IsState(State st)
    {
        return (_st == st) ? true : false;
    }
    
    bool IsNotState(State st)
    {
        return (_st != st) ? true : false;
    }
    void ChangeState(State end_state, T* ptr, bool (T::*fun)())
    {
        bool lbRet = (ptr->*fun)();
        
        if(lbRet) {
            _st = end_state;
        }
    }
    
private:
    State _st;
};

#endif //_COMMON_H_




