/********************************************************************
 * author: isold.wang@gmail.com
*********************************************************************/
#include "common.h"
uint32_t CCommon::GetProcessId()
{
    pid_t pid = getpid();
    uint32_t id = pid;
    return id;
}

uint32_t CCommon::GetProcessParentId()
{
    pid_t pid = getppid();
    uint32_t id = pid;
    return id;
}

void CCommon::SetOpenFiles(uint32_t max_file_opens_number)
{
    //修改core文件大小限制和进程最大文件句柄数
    struct rlimit rlimit_file_new;
    struct rlimit rlimit_file_old;
    
    if(getrlimit(RLIMIT_NOFILE, &rlimit_file_old) == 0) {
        rlimit_file_new.rlim_cur = max_file_opens_number;
        rlimit_file_new.rlim_max = max_file_opens_number;
        
        if(setrlimit(RLIMIT_NOFILE, &rlimit_file_new) != 0) {
            int errorcode = errno;
            
            if(errorcode == EFAULT) {
                return;
            } else if(errorcode == EINVAL) {
                return;
            } else if(errorcode == EPERM) {
                return;
            }
            
            rlimit_file_new.rlim_cur = rlimit_file_old.rlim_cur;
            rlimit_file_new.rlim_max = rlimit_file_old.rlim_max;
            setrlimit(RLIMIT_NOFILE, &rlimit_file_new);
        }
    }
}


//得到当前应用程序的路径
void CCommon::GetAppPath(char* apPath, int aiLen)
{
    memset(apPath, 0, aiLen);
#ifdef WIN32
    GetModuleFileName(NULL, (LPSTR)apPath, aiLen);
#else
    char lcAppPath[256 + 1] = "";
    sprintf(lcAppPath, "/proc/self/exe");
    readlink(lcAppPath, apPath, aiLen - 1);
#endif
}

//创建路径
void CCommon::CreatePath(char* apPath)
{
#ifdef WIN32
    CreateDirectory(apPath, NULL);
#else
    //mkdir函数的第二个参数用于设置创建文件夹的权限，
    //权限采用8进制，设置为777表示所有用户都可以查看
    mkdir(apPath, 511);
#endif
}


uint64_t CCommon::get_current_time()
{
    struct timeb loTimeb;
    memset(&loTimeb, 0 , sizeof(timeb));
    ftime(&loTimeb);
    return ((uint64_t)loTimeb.time * 1000) + loTimeb.millitm;
}

uint64_t CCommon::escape_time(uint64_t last_time)
{
    return 0;
}

std::string CCommon::get_now()
{
    std::string strCurrentTime = "";
    struct timeb loSystemTime;
    ftime(&loSystemTime);
    struct tm* lptm = localtime(&loSystemTime.time);
    stringstream lstrTimeNow;
    lstrTimeNow.fill('0');
    lstrTimeNow << setw(4) << 1900 + lptm->tm_year << "-" <<
                setw(2) << 1 + lptm->tm_mon << "-" <<
                setw(2) << lptm->tm_mday << " " <<
                setw(2) << lptm->tm_hour << ":" <<
                setw(2) << lptm->tm_min << ":" <<
                setw(2) << lptm->tm_sec << ":" <<
                setw(3) << loSystemTime.millitm << ends;
    strCurrentTime = lstrTimeNow.str();
    return strCurrentTime;
}

//检测是否超时
bool CCommon::CheckTimeOut(const int64_t& ai64LastTime, const int64_t& ai64TimeInterval)
{
    int64_t now = CCommon::get_current_time();
    
    if(now > ai64LastTime + ai64TimeInterval) {
        return true;
    }
    
    return false;
}

//根据IP地址和包序号计算主键
int64_t CCommon::MakeIpPortKey(const sockaddr_in& addr, const uint16_t awPackSerial)
{
#ifdef WIN32
    return (int64_t)addr.sin_addr.S_un.S_addr * 0x100000000 + \
           (int64_t)addr.sin_port * 0x10000 + awPackSerial;
#else
    return (int64_t)addr.sin_addr.s_addr * 0x100000000LL + \
           (int64_t)addr.sin_port * 0x10000 + awPackSerial;
#endif
}

//通过sockaddr_in得到int型的IP地址
uint32_t CCommon::GetSocketAddr(const sockaddr_in& addr)
{
#ifdef WIN32
    return addr.sin_addr.S_un.S_addr;
#else
    return addr.sin_addr.s_addr;
#endif
}

void CCommon::GetSocketAddr(const sockaddr_in& addr, uint32_t& ip, uint16_t& port)
{
    ip = ntohl(addr.sin_addr.s_addr);
    port = ntohs(addr.sin_port);
}

uint32_t CCommon::GetSocketAddr(const char* addr)
{
    return ntohl(inet_addr(addr));
}

//把int型的IP地址设置到sockaddr_in结构体中
void CCommon::SetSocketAddr(sockaddr_in& addr, uint32_t alAddr)
{
#ifdef WIN32
    addr.sin_addr.S_un.S_addr = alAddr;
#else
    addr.sin_addr.s_addr = alAddr;
#endif
}

void CCommon::SetSocketAddr(sockaddr_in& addr, uint32_t alAddr, uint16_t alPort)
{
    addr.sin_family = AF_INET;
    addr.sin_port = alPort;
#ifdef WIN32
    addr.sin_addr.S_un.S_addr = alAddr;
#else
    addr.sin_addr.s_addr = alAddr;
#endif
}

//从int型的IP地址得到字符串类型的IP地址
char* CCommon::GetIPAddr(uint32_t alAddr)
{
    sockaddr_in addr;
#ifdef WIN32
    addr.sin_addr.S_un.S_addr = alAddr;
#else
    addr.sin_addr.s_addr = alAddr;
#endif
    return inet_ntoa(addr.sin_addr);
}

//从time_t类型取得日期和时间的字符串形式
char* CCommon::Getctime(time_t* apTime)
{
    static char lszTime[20];
    strftime(lszTime, 20, "%Y-%m-%d %H:%M:%S", localtime(apTime));
    return lszTime;
}


//封装输出调用堆栈的功能
void CCommon::DumpStack()
{
    void* larray[25];
    int liSize = backtrace(larray, 25);
    char** lszSymbols = backtrace_symbols(larray, liSize);
    
    for(int i = 0; i < liSize; i++) {
        TRACE(1, "CUCTools::DumpStack " << lszSymbols[i]);
    }
    
    if(lszSymbols)
        free(lszSymbols);
}

uint32_t CCommon::GetRandInt(uint32_t rand_max)
{
    ASSERT(rand_max > 1);
    return 1 + (int)((float)rand_max * (rand() / (RAND_MAX + 1.0)));
}

std::string CCommon::ChgStrInRandomSeq(const std::string& astrData, int aiChgTimes)
{
    ASSERT(aiChgTimes > 0 && astrData.size() > 1);
    std::string lstrNewData = astrData;
    
    for(int i = 0; i < aiChgTimes; ++i) {
        std::string lstr1 = lstrNewData.substr(0, lstrNewData.size() / 2);
        lstrNewData = lstrNewData.substr(lstrNewData.size() / 2);
        uint32_t liInsertPos = 0;
        
        for(std::string::size_type i = 0; i < lstr1.size(); ++i) {
            liInsertPos = GetRandInt(lstrNewData.size() + 1) - 1;
            
            if(liInsertPos < lstrNewData.size()) {
                lstrNewData.insert(liInsertPos, 1, lstr1.at(i));
            } else {
                lstrNewData += lstr1.at(i);
            }
        }
    }
    
    return lstrNewData;
}

uint8_t CCommon::GetPasswd(const char* lszUserName, struct Passwd* opPasswd)
{
    if(lszUserName != NULL && opPasswd != NULL) {
        if(getpwnam_r(lszUserName, &opPasswd->moPd, opPasswd->mcbuffer, Passwd::BUFF_LEN, &opPasswd->motmpPwd) != 0) {
            perror("CCommon::GetPasswd getpwnam_r() error.");
            return FAIL;
        } else {
            return SUCC;
        }
    } else {
        return FAIL;
    }
}

void CCommon::SetUA2SomeBody(const char* lszUserName)
{
    if(geteuid() == 0) {
        if(setgid((gid_t)0) == -1) {
            perror("CCommon::SetUA2SomeBody setgid.");
            /* fatal */
            exit(2);
        }
        
        if(initgroups(lszUserName, (gid_t)getgid()) == -1) {
            perror("CCommon::SetUA2SomeBody initgroups.");
        }
        
        struct  Passwd oPasswd;
        
        if(GetPasswd(lszUserName, &oPasswd) == SUCC) {
            if(setuid(oPasswd.moPd.pw_uid) == -1) {
                perror("CCommon::SetUA2SomeBody setuid.");
                /* fatal */
                exit(2);
            }
        } else {
            perror("CCommon::SetUA2SomeBody GetPasswd.");
            exit(2);
        }
    }
}

uint8_t CCommon::GetCurWorkingPath(char* oszcPath, int ilen)
{
    if(getcwd(oszcPath, ilen) == 0) {
        return SUCC;
    } else {
        perror("CCommon::GetCurWorkingPath getcwd.");
        return FAIL;
    }
}

