/***********************************************************
file name: DebugTrace.h
auther: isold.wang@gmail.com
***********************************************************/

#ifndef DEF_DEBUGTRACE_HEAD_
#define DEF_DEBUGTRACE_HEAD_

//the mark of log
#define INFORMATION_OUTPUT
//max size of per line
#define DEF_MAX_BUFF_LEN 4096

#ifdef _DIRECT
//direct output to file when in debug
#define LOG_OUTPUT_TO_FILE_DIRECT
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <stdarg.h>
#endif

#include <sstream>
#include <string>
using std::stringstream;

#include "critical_section.h"
#include "sigslot.h"

class CDebugTrace;
extern CDebugTrace* goDebugTrace;

#undef ASSERT       //取消ASSERT宏
//  重新定义ASSERT宏
#ifdef _DEBUG       //调试版本使用此宏
#define ASSERT(f)               \
    if(f)                           \
    ;                           \
    else                            \
    goDebugTrace->AssertFail(#f,__FILE__, __LINE__)
#else
#define ASSERT(f)
#endif//_DEBUG

#define SET_TRACE_LEVEL     goDebugTrace->SetTraceLevel
#define SET_LOG_FILENAME    goDebugTrace->SetLogFileName
#define SET_TRACE_OPTIONS   goDebugTrace->SetTraceOptions
#define GET_TRACE_OPTIONS   goDebugTrace->GetTraceOptions

#undef TRACE

//trace when level is bigger
#ifdef INFORMATION_OUTPUT
#define TRACE(level, args) \
    if (!goDebugTrace->CanTrace(level))     ;  else\
    {\
        goDebugTrace->Lock();\
        try\
        {\
            goDebugTrace->BeginTrace(level, __FILE__, __LINE__);\
            *goDebugTrace << args << '\n';\
            goDebugTrace->EndTrace();\
        }\
        catch (...)\
        {\
        }\
        goDebugTrace->UnLock();\
    }
#else
#define TRACE ;
#endif //INFORMATION_OUTPUT

#define TRACE_FORMAT(level, _fmt_, ...) \
    if (!goDebugTrace->CanTrace(level))     ;  else\
    {\
        goDebugTrace->Lock();\
        try\
        {\
            goDebugTrace->BeginTrace(level, __FILE__, __LINE__);\
            goDebugTrace->TraceFormat(_fmt_, ##__VA_ARGS__);\
            *goDebugTrace<<"\n";\
            goDebugTrace->EndTrace();\
        }\
        catch (...)\
        {\
        }\
        goDebugTrace->UnLock();\
    }

//trace key id
extern int64_t gi64TraceUserId;

#define SET_TRACE_USER_ID(userid)  (gi64TraceUserId=(userid));\
{\
    goDebugTrace->Lock();\
    try\
    {\
        (goDebugTrace->BeginTrace(0,__FILE__,__LINE__) << " key id: " << userid << '\n').EndTrace();\
    }\
    catch (...)\
    {\
    }\
    goDebugTrace->UnLock();\
}

#define TraceUserEvent(userid, args) \
{\
    goDebugTrace->Lock();\
    try\
    {\
        if ((userid) == gi64TraceUserId)\
        (goDebugTrace->BeginTrace(0,__FILE__,__LINE__) << args << '\n').EndTrace();\
    }\
    catch (...)\
    {\
    }\
    goDebugTrace->UnLock();\
}

#define LOG_DEBUG(_fmt_, ...) TRACE_FORMAT(5, _fmt_, ##__VA_ARGS__)
#define LOG_TRACE(_fmt_, ...) TRACE_FORMAT(4, _fmt_, ##__VA_ARGS__)
#define LOG_NOTICE(_fmt_, ...) TRACE_FORMAT(3, _fmt_, ##__VA_ARGS__)
#define LOG_WARNING(_fmt_, ...) TRACE_FORMAT(2, _fmt_, ##__VA_ARGS__)
#define LOG_FATAL(_fmt_, ...) TRACE_FORMAT(1, _fmt_, ##__VA_ARGS__)

//日志实现类声明
typedef CDebugTrace& (* DebugTraceFunc)(CDebugTrace& aoDebugTrace);

class CDebugTrace
{
public:
    //打印选项
    enum Options {
        /// 打印时间
        Timestamp = 1,
        /// 打印日志级别
        LogLevel = 2,
        /// 打印源文件名和行号
        FileAndLine = 4,
        /// 把日志追加到文件中
        AppendToFile = 8,
        ///输出日志到控制台
        PrintToConsole = 16
    };
    
public:
    //构造函数
    CDebugTrace(unsigned asTraceOptions = Timestamp | AppendToFile | FileAndLine | LogLevel);
    //析够函数
    ~CDebugTrace();
public:
    //设置日志级别(0级最高,1其次,依次类推,小于该等级的日志不打印)
    void SetTraceLevel(int aiTraceLevel);
    
    //设置日志文件最大字节数
    void SetLogFileSize(uint32_t aiSize);
    
    //设置日志文件名
    void SetLogFileName(char* aszLogFile);
    
    void SetTraceOptions(unsigned int options /** New level for trace */);
    
    unsigned GetTraceOptions(void);
    
    inline bool CanTrace(int aiLogLevel)
    {
        return (aiLogLevel <= mnLogLevel) && (aiLogLevel);
    }
    
    CDebugTrace& BeginTrace(int aiLogLevel, const char* apSrcFile, int aiSrcLine);
    
    void EndTrace();
    
    void AssertFail(const char* strCond, const char* strFile, unsigned uLine);
    
    void TraceFormat(const char* format, ...);
    
    void TraceStack(void);
    
    CDebugTrace& endl(CDebugTrace& _outs);
    
    template <class T>
    inline CDebugTrace& operator<< (const T value)
    {
        stringstream str;
        str << value;
        std::string ss = "";
        ss = str.str();
        
        if(mlDataLen < DEF_MAX_BUFF_LEN - (int)ss.length()) {
            memcpy((void*)(mszPrintBuff + mlDataLen), ss.c_str(), ss.length());
            mlDataLen += ss.length();
        }
        
        return *this;
    }
    
    template <size_t size>
    CDebugTrace& operator << (const char (&apStrVal)[size])
    {
        char* lpWritePtr = mszPrintBuff + mlDataLen;
        
        if(apStrVal == 0) {
            if(mlDataLen < (int)(DEF_MAX_BUFF_LEN - strlen("NULL")))
                mlDataLen += sprintf(lpWritePtr, "%s", "NULL");
        } else {
            if(mlDataLen < (int)(DEF_MAX_BUFF_LEN - strlen(apStrVal)))
                mlDataLen += sprintf(lpWritePtr, "%s", apStrVal);
        }
        
        return *this;
    }
    
    CDebugTrace& operator << (const uint8_t acCharVal);
    CDebugTrace& operator << (const bool abBoolVal);
    CDebugTrace& operator << (const char* apStrVal);
    CDebugTrace& operator << (const std::string& apStrVal);
    CDebugTrace& operator << (DebugTraceFunc _f)
    {
        (*_f)(*this);
        return *this;
    }
    
    //输出和清空临时缓冲区
    int Flush();
    void Lock()
    {
        moCriticalSection.Enter();
    }
    void UnLock()
    {
        moCriticalSection.Leave();
    }
    
private:
    uint32_t GetFileSize(FILE* fp)
    {
        ASSERT(fp != NULL);
        uint32_t cur_pos = 0;
        uint32_t len = 0;
        cur_pos = ftell(fp);
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, cur_pos, SEEK_SET);
        return len;
    }
    bool    AddToLogBuffer();
    enum { LOG_BUFFER_LEN = 1024 * 512 };
public:
    char        mszPrintBuff[DEF_MAX_BUFF_LEN + 1];  //打印数据缓存
    int     mlDataLen;           //数据长度
    int         mnLogLevel;          //日志等级
    char        mszLogFileName[512]; //日志文件名称
    char mszLogFileNamePre[450];
    unsigned int muTraceOptions;      //打印日志选项
    
    CCriticalSection moCriticalSection;     //打印日志同步临界区
    struct timeb moSystemTime;              //某次会话的时间
    int  miLogLevel;                        //某次会话的日志等级
    uint32_t muiLogFileSize;
    
private:
    char    macMark[20];
    int    mlBufDataLen;
    char    mszLogBuffer[LOG_BUFFER_LEN + 1];
};
#endif  //DEF_DEBUGTRACE_HEAD_
