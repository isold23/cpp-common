/***********************************************************
 * author: isold.wang@gmail.com
***********************************************************/
#include <stdio.h>
#include "include.h"
#include "debugtrace.h"
#include "file_stream.h"

char goLogMark[] = "SERVER_LOG_BEGIN";

int64_t gi64TraceUserId;

CDebugTrace::CDebugTrace(unsigned asTraceOptions)
{
    mlDataLen = 0;
    mnLogLevel = 4;
    memset(mszLogFileName, 0, 512);
    memset(mszLogFileNamePre, 0, 450);
    memset(mszPrintBuff, 0, sizeof(mszPrintBuff));
    muTraceOptions = asTraceOptions;
    muiLogFileSize = 1024 * 1024 * 1024;
#ifndef LOG_OUTPUT_TO_FILE_DIRECT
    ASSERT(LOG_BUFFER_LEN > DEF_MAX_BUFF_LEN * 5);
    strncpy(macMark, goLogMark, sizeof(macMark));
    mlBufDataLen = 0 ;
#endif
}

CDebugTrace::~CDebugTrace()
{
#ifndef LOG_OUTPUT_TO_FILE_DIRECT
    Flush();
#endif //LOG_OUTPUT_TO_FILE_DIRECT
}

//print char
CDebugTrace& CDebugTrace::operator << (const uint8_t acCharVal)
{
    if(mlDataLen < DEF_MAX_BUFF_LEN - 2) {
        char* lpWritePtr = mszPrintBuff + mlDataLen;
        mlDataLen += sprintf(lpWritePtr, "%d", acCharVal);
    }
    
    return *this;
}

//print bool
CDebugTrace& CDebugTrace::operator << (const bool abBoolVal)
{
    if(mlDataLen < DEF_MAX_BUFF_LEN - 6) {
        char* lpWritePtr = mszPrintBuff + mlDataLen;
        
        if(abBoolVal) {
            mlDataLen += sprintf(lpWritePtr, "%s", "true");
        } else {
            mlDataLen += sprintf(lpWritePtr, "%s", "false");
        }
    }
    
    return *this;
}

CDebugTrace& CDebugTrace::operator << (const char* apStrVal)
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

CDebugTrace& CDebugTrace::operator << (const std::string& apStrVal)
{
    char* lpWritePtr = mszPrintBuff + mlDataLen;
    
    if(apStrVal.empty()) {
        if(mlDataLen < (int)(DEF_MAX_BUFF_LEN - strlen("NULL")))
            mlDataLen += sprintf(lpWritePtr, "%s", "NULL");
    } else {
        if(mlDataLen < (int)(DEF_MAX_BUFF_LEN - apStrVal.length()))
            mlDataLen += sprintf(lpWritePtr, "%s", apStrVal.c_str());
    }
    
    return *this;
}


//inline _CRTIMP ostream& __cdecl endl(ostream& _outs) { return _outs << '\n' << flush; }
CDebugTrace& CDebugTrace::endl(CDebugTrace& aoDebugTrace)
{
#ifdef WIN32
    OutputDebugString(aoDebugTrace.mszPrintBuff);
#endif
    
    if(aoDebugTrace.muTraceOptions & CDebugTrace::PrintToConsole) {
        printf("%s", aoDebugTrace.mszPrintBuff);
    }
    
    if((aoDebugTrace.muTraceOptions & CDebugTrace::AppendToFile) && (strlen(aoDebugTrace.mszLogFileName) > 1)) {
        FILE* lfpTraceFile = NULL;
        lfpTraceFile = fopen(aoDebugTrace.mszLogFileName, "a");
        
        if(lfpTraceFile != NULL) {
            fprintf(lfpTraceFile, "%s", aoDebugTrace.mszPrintBuff);
            fclose(lfpTraceFile);
        }
    }
    
    aoDebugTrace.mlDataLen = 0;
    moCriticalSection.Leave();
    return aoDebugTrace;
}

void CDebugTrace::TraceFormat(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
#ifdef WIN32
    mlDataLen += _vsnprintf(mszPrintBuff + mlDataLen,
                            DEF_MAX_BUFF_LEN - mlDataLen,
                            format, argptr);
#else
    mlDataLen += vsnprintf(mszPrintBuff + mlDataLen,
                           DEF_MAX_BUFF_LEN - mlDataLen,
                           format, argptr);
#endif
    va_end(argptr);
}

void CDebugTrace::SetTraceLevel(int aiTraceLevel)
{
    mnLogLevel = aiTraceLevel;
}

void CDebugTrace::SetLogFileSize(uint32_t aiSize)
{
    muiLogFileSize = aiSize;
}

void CDebugTrace::SetLogFileName(char* aszLogFile)
{
#ifndef LOG_OUTPUT_TO_FILE_DIRECT
    Flush();
#endif //LOG_OUTPUT_TO_FILE_DIRECT
    
    if(aszLogFile != NULL) {
        ASSERT(strlen(aszLogFile) <= 512);
        strcpy(mszLogFileNamePre, aszLogFile);
    } else {
        strcpy(mszLogFileNamePre, "");
    }
    
    char lszFileDate[50];
    memset(lszFileDate, 0, 50);
    time_t loSystemTime;
    time(&loSystemTime);
    struct tm* lptm = localtime(&loSystemTime);
    sprintf(lszFileDate, "-%4d%02d%02d%02d%02d.log",
            1900 + lptm->tm_year, 1 + lptm->tm_mon, lptm->tm_mday,
            lptm->tm_hour, lptm->tm_min);
    memcpy(mszLogFileName, mszLogFileNamePre, strlen(mszLogFileNamePre));
    memcpy(mszLogFileName + strlen(mszLogFileNamePre), lszFileDate, strlen(lszFileDate));
}

void CDebugTrace::SetTraceOptions(unsigned int options /** New level for trace */)
{
    muTraceOptions = options;
}

unsigned CDebugTrace::GetTraceOptions(void)
{
    return muTraceOptions;
}

CDebugTrace& CDebugTrace::BeginTrace(int aiLogLevel, const char* apSrcFile, int aiSrcLine)
{
    mlDataLen = 0;  //已打印的数据长度清0
    memset(mszPrintBuff, 0, (DEF_MAX_BUFF_LEN + 1));
    miLogLevel = aiLogLevel;
    ftime(&moSystemTime);
    struct tm* lptm = localtime(&moSystemTime.time);
    
    //time (hour:min:sec:misec)
    if(muTraceOptions & Timestamp) {
        char lszTraceDataBuff[20];
        sprintf(lszTraceDataBuff, "%02d:%02d:%02d:%03d", \
                lptm->tm_hour, lptm->tm_min, lptm->tm_sec, moSystemTime.millitm);
        *this << lszTraceDataBuff << ' ';
    }
    
    //level
    if(muTraceOptions & LogLevel) {
        switch(aiLogLevel) {
        case 1:
            *this << "\033[31;1m FATAL" << ' ';
            break;
            
        case 2:
            *this << "\033[33;1m WARNING" << ' ';
            break;
            
        case 3:
            *this << "\033[35;1m NOTICE" << ' ';
            break;
            
        case 4:
            *this << "\033[34;1m TRACE" << ' ';
            break;
            
        case 5:
            *this << "\033[36;1m DEBUG" << ' ';
            break;
            
        default:
            break;
        }
    }
    *this << " \33[0m ";
    
    //file name and line number and function name
    if(muTraceOptions & FileAndLine) {
        *this << apSrcFile << "[" << aiSrcLine << "] " << " ";
    }
    
    return *this;
}

void CDebugTrace::EndTrace()
{
    try {
#ifdef WIN32
        //输出到调试窗口中
        OutputDebugString(mszPrintBuff);
#endif
        
        //若要求输出到控制台,则把日志信息在控制台也打印一下
        if(muTraceOptions & PrintToConsole) {
            printf("%s" , mszPrintBuff);
        }
        
        //若要求写文件且设置了日志文件名,则把日志信息写入文件中
        if((muTraceOptions & AppendToFile) \
                && (strlen(mszLogFileName) > 1)) {
            //fanyunfeng 直接输出到文件或者输出到临时缓冲区
#ifdef LOG_OUTPUT_TO_FILE_DIRECT
            FILE* lfpTraceFile = NULL;
            lfpTraceFile = fopen(mszLogFileName, "a");
            
            if(lfpTraceFile != NULL) {
                if(GetFileSize(lfpTraceFile) > muiLogFileSize) {
                    SetLogFileName(mszLogFileNamePre);
                    fclose(lfpTraceFile);
                    lfpTraceFile = NULL;
                    lfpTraceFile = fopen(mszLogFileName, "a");
                }
            }
            
            if(lfpTraceFile != NULL) {
                fprintf(lfpTraceFile, "%s", mszPrintBuff);
                fclose(lfpTraceFile);
            }
            
#else  //LOG_OUTPUT_TO_FILE_DIRECT
            AddToLogBuffer();
#endif //LOG_OUTPUT_TO_FILE_DIRECT
        }
    } catch(...) {
    }
}

void CDebugTrace::AssertFail(const char* strCond, const char* strFile, unsigned uLine)
{
    char szMessage[512];
    char strExePath[256];
    CCommon::GetAppPath(strExePath, 200);
#ifdef WIN32
    sprintf(szMessage, " Debug Assertion Failed!\n Program: %s   \n File: %s  \n Condition: ASSERT(%s);    \n Line:%d \n\n Continue?",
            strExePath, strFile, strCond, uLine);
    int nResult = MessageBox(NULL, szMessage, "Assert failure", MB_OKCANCEL + MB_ICONERROR);
    sprintf(szMessage, " Debug Assertion Failed!\n File: %s Line:%d Condition: ASSERT(%s); \n", strFile, uLine, strCond);
    OutputDebugString(szMessage);
    
    if(nResult == IDCANCEL) {
        FatalExit(-1);
    }
    
    DebugBreak();
#else
    sprintf(szMessage, " Debug Assertion Failed!\n Program: %s   \n File: %s  \n Condition: ASSERT(%s);    \n Line:%d \n\n",
            strExePath, strFile, strCond, uLine);
    TRACE(1, szMessage);
    exit(0);
#endif
}

int CDebugTrace::Flush()
{
    int liRet = 0;
#ifndef  LOG_OUTPUT_TO_FILE_DIRECT
    CAutoLock loLock(moCriticalSection);
    
    try {
        if(mlBufDataLen > 0) {
            FILE* lfpTraceFile = NULL;
            lfpTraceFile = fopen(mszLogFileName, "a");
            
            if(lfpTraceFile != NULL) {
                if(GetFileSize(lfpTraceFile) > muiLogFileSize) {
                    SetLogFileName(mszLogFileNamePre);
                    fclose(lfpTraceFile);
                    lfpTraceFile = NULL;
                    lfpTraceFile = fopen(mszLogFileName, "a");
                }
            }
            
            if(lfpTraceFile != NULL) {
                if(1 != fwrite(mszLogBuffer, mlBufDataLen, 1, lfpTraceFile)) {
                    liRet = errno;
                }
                
                fclose(lfpTraceFile);
                //clear buffer
                mlBufDataLen = 0;
                memset(mszLogBuffer, 0, (LOG_BUFFER_LEN + 1));
            } else {
                liRet = errno;
            }
        }
    } catch(...) {
        liRet = errno;
    }
    
#endif // LOG_OUTPUT_TO_FILE_DIRECT
    return liRet;
}

#ifndef  LOG_OUTPUT_TO_FILE_DIRECT
bool CDebugTrace::AddToLogBuffer()
{
    int liFlushRet = 0;
    
    if(mlDataLen > LOG_BUFFER_LEN - mlBufDataLen) {
        if(NULL != (liFlushRet = Flush())) {
            //清空所有的缓冲区
            mlBufDataLen = 0;
            //增加一条调试信息
            struct timeb loSystemTime;
            ftime(&loSystemTime);
            struct tm* lptm = localtime(&loSystemTime.time);
            mlBufDataLen = sprintf(mszLogBuffer, "%02d:%02d:%02d:%03d ***CDebugTrace::AddToLogBuffer 输出缓冲区时失败 日志发生循环覆盖(ENO:%d)\n",
                                   lptm->tm_hour, lptm->tm_min, lptm->tm_sec, loSystemTime.millitm, liFlushRet);
        }
    }
    
    //前面已经确认过缓冲区的长度
    //缓冲区足够长
    memcpy(mszLogBuffer + mlBufDataLen, mszPrintBuff, mlDataLen);
    mlBufDataLen += mlDataLen;
    return true;
}
#endif // LOG_OUTPUT_TO_FILE_DIRECT

//打印当前堆栈
void CDebugTrace::TraceStack(void)
{
    /*
    bool bResult = false;
    HANDLE         hProcess = GetCurrentProcess();
    HANDLE         hThread = GetCurrentThread();
    
    for( uint32_t index = 0; ; index++ )
    {
    bResult = StackWalk(
    IMAGE_FILE_MACHINE_I386,
    hProcess,
    hThread,
    &callStack,
    NULL,
    NULL,
    SymFunctionTableAccess,
    SymGetModuleBase,
    NULL);
    
    if ( index == 0 )
    continue;
    
    if( !bResult || callStack.AddrFrame.Offset == 0 )
    break;
    
    GetFunctionInfoFromAddresses( callStack.AddrPC.Offset, callStack.AddrFrame.Offset, symInfo );
    GetSourceInfoFromAddress( callStack.AddrPC.Offset, srcInfo );
    
    fputs("    ", lfpTraceFile);
    fputs(srcInfo, lfpTraceFile);
    fputs("\n      ", lfpTraceFile);
    fputs(symInfo, lfpTraceFile);
    fputs("\n", lfpTraceFile);
    }
    */
}
