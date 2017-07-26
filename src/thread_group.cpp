#include "thread_group.h"

CThreadGroup::CThreadGroup(void)
    : moStopEvent("ThreadGroupStopEvent")
{
    mbStop = true;
}

CThreadGroup::~CThreadGroup(void)
{
    if(false == mbStop) {
        StopAll();
    }
}

bool CThreadGroup::Sleep(uint32_t ldwWaitTime)
{
    if(false == mbStop) {
        mbStop = true;
        int miResult = moStopEvent.WaitForEvent(ldwWaitTime);
        
        if(0 == miResult || -1 == miResult) {
            return true;
        }
    }
    
    return false;
}

void CThreadGroup::WakeAll()
{
    if(mbStop) {
        moStopEvent.SetEvent();
        mbStop = false;
    }
}

uint32_t CThreadGroup::Start(ThreadFunc* apThreadFunc, \
                             void* apWorkContext, \
                             uint32_t auiThreadNum, \
                             char* apThreadName, \
                             int aiPriority)
{
    ASSERT(apThreadFunc);
    STRU_THREAD_CONTEXT* lpThreadContext = NULL;
    uint32_t luiCounter = 0;
    CAutoLock loLock(moAccessCS);
    
    //如果是停止状态,重新设置停止标记
    if(mbStop) {
        mbStop = false;
        
        if(!moStopEvent.Create(false, false)) {
            TRACE(1, "CThreadGroup::Initialize 创建事件失败。");
            return luiCounter;
        }
    }
    
    try {
        for(luiCounter = 0; luiCounter < auiThreadNum; luiCounter++) {
            lpThreadContext = NULL;
            lpThreadContext = new STRU_THREAD_CONTEXT();
            lpThreadContext->mpGroupManager = this;
            lpThreadContext->mpWorkContext = apWorkContext;
            lpThreadContext->moThreadStat.SetThreadName(apThreadName, luiCounter);
            lpThreadContext->miPriority = aiPriority;
            
            if(StartThread(lpThreadContext, apThreadFunc)) {
                moThreadList.push_back(lpThreadContext);
                lpThreadContext = NULL;
            } else {
                delete lpThreadContext;
                lpThreadContext = NULL;
            }
        }
    } catch(...) {
        //删除创建的环境对象
        TRACE(1, "CThreadGroup::Start 创建线程失败。");
        
        if(lpThreadContext) {
            delete lpThreadContext;
            lpThreadContext = NULL;
        }
    }
    
    return luiCounter;
}

//启动线程
#ifdef WIN32
bool CThreadGroup::StartThread(STRU_THREAD_CONTEXT* apContext, ThreadFunc* apThreadFunc)
{
    apContext->mhHandle = (HANDLE)_beginthreadex(NULL, NULL, \
                          (StdThreadFunc*)apThreadFunc, apContext, NULL, \
                          & (apContext->muiThreadID));
                          
    //当系统创建线程达到极限时，返回值为0，创建失败返回-1
    if(apContext->mhHandle > 0) {
        //设置线程优先级
        SetThreadPriority((HANDLE)apContext->mhHandle, apContext->miPriority);
        return true;
    }
    
    TRACE(1, "CThreadGroup::StartThread 创建线程失败 ERROR_NO:" << GetLastError());
    return false;
}
#else
bool CThreadGroup::StartThread(STRU_THREAD_CONTEXT* apContext, ThreadFunc* apThreadFunc)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // 设置分离线程线程，优点：1、速度很快;2、自动释放资源
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, 2000000);  //设置线程分配堆栈的大小，单位：字节
    int liResult = pthread_create(& (apContext->muiThreadID), &attr, \
                                  (StdThreadFunc*)apThreadFunc, apContext);
    pthread_attr_destroy(&attr);
    
    if(0 == liResult) {
        apContext->muiThreadID = pthread_self();
        //设置线程优先级
        //pthread_setschedparam(apContext->muiThreadID, apContext->miPriority, NULL);
        //取得线程创建时间
        memset(&apContext->moStartTime, 0, sizeof(apContext->moStartTime));
        ftime(&apContext->moStartTime);
        liResult = pthread_detach(apContext->muiThreadID);
        //TRACE(1, "CThreadGroup::StartThread pthread_detach 返回值: "<<liResult);
        return true;
    }
    
    TRACE(1, "CThreadGroup::StartThread 创建线程失败。原因:" << liResult);
    return false;
}
#endif

bool CThreadGroup::StopAll()
{
    int liThread = (int)moThreadList.size();
    list<STRU_THREAD_CONTEXT*>::iterator loIterator;
    int liTermainte = 0;
    
    if(!IsStop()) {
        CAutoLock loLock(moAccessCS);
        mbStop = true;  //重新设置停止标记
        //设置退出事件,使收有的Sleep返回失败
        moStopEvent.SetEvent();
        
        //等待所有的线程退出
        for(loIterator = moThreadList.begin(); loIterator != moThreadList.end(); loIterator++) {
            if(!EndThread(*loIterator)) {
                liTermainte++;
            }
            
            delete(*loIterator);
        }
        
        //清除所有的线程信息
        moThreadList.clear();
        //退出事件
        moStopEvent.Close();
    }
    
    if(liTermainte) {
        TRACE(1, "CThreadGroup::StopAll 强行结束线程数量" << liTermainte
              << " 正常结束数量:" << liThread - liTermainte);
    } else {
        TRACE(1, "CThreadGroup::StopAll 所有线程正常结束。");
    }
    
    return true;
}

//停止线程
#ifdef WIN32
bool CThreadGroup::EndThread(STRU_THREAD_CONTEXT* apContext)
{
    bool lbResult = true;
    
    if(WaitForSingleObject(apContext->mhHandle, 2000) != WAIT_OBJECT_0) {
        TRACE(1, "CThreadGroup::EndThread 强行结束线程" << apContext->moThreadStat.GetThreadName());
        TerminateThread(apContext->mhHandle, 0xFFFFFFFF);
        lbResult = false;
    }
    
    CloseHandle(apContext->mhHandle);
    return lbResult;
}

#else
bool CThreadGroup::EndThread(STRU_THREAD_CONTEXT* apContext)
{
    bool lbResult = true;
    int liWaitCount = 0;
    
    while((liWaitCount++ < 12)) {  // && (0 == pthread_kill(apContext->muiThreadID, 0)))
        usleep(99000);  //微秒级，约100ms
    }
    
    //if (0 == pthread_kill(apContext->muiThreadID, 0))
    //{
    //  TRACE(1,"CThreadGroup::EndThread 强行结束线程:" << apContext->moThreadStat.GetThreadName()
    //      << " ID:" << apContext->muiThreadID);
    //  pthread_cancel(apContext->muiThreadID);
    //  lbResult = false;
    //}
    return lbResult;
}
#endif

void CThreadGroup::Dump()
{
    list<STRU_THREAD_CONTEXT*>::iterator loIterator;
    CAutoLock loLock(moAccessCS);
    TRACE(1, "CThreadGroup::Dump 线程数量:" << moThreadList.size());
    
    for(loIterator = moThreadList.begin(); loIterator != moThreadList.end(); loIterator++) {
        PrintThreadInfo(**loIterator);
    }
}

#ifdef WIN32
void CThreadGroup::PrintThreadInfo(STRU_THREAD_CONTEXT& aoThreadContext)
{
    ASSERT(&aoThreadContext != NULL);
    //打印线程信息
    TRACE(1, "CThreadGroup::Dump 线程ID:" << aoThreadContext.muiThreadID
          << "句柄:" << aoThreadContext.mhHandle
          << "线程名称:" << aoThreadContext.moThreadStat.GetThreadName()
          << "性能:" << aoThreadContext.moThreadStat.GetPerformance()
          << "优先级:" << aoThreadContext.miPriority);
    FILETIME lstruCreationTime;
    FILETIME lstruExitTime;
    FILETIME lstruKernelTime;
    FILETIME lstruUserTime;
    
    if(GetThreadTimes(aoThreadContext.mhHandle, &lstruCreationTime, &lstruExitTime, &lstruKernelTime, &lstruUserTime)) {
        PrintThreadTime("创建时间", lstruCreationTime);
        uint32_t    ldwExitCode = NULL;
        
        //输出退出时间
        if(GetExitCodeThread(aoThreadContext.mhHandle, &ldwExitCode)) {
            if(ldwExitCode != STILL_ACTIVE) {
                char lszExit[32];
                sprintf(lszExit, "退出代码%u 时间", ldwExitCode);
                PrintThreadTime(lszExit, lstruExitTime);
            } else {
                TRACE(1, "CThreadGroup::Dump 线程没有退出，存活计数:"
                      << aoThreadContext.moThreadStat.GetAliveCounter());
            }
        }
        
        PrintThreadTime("内核时间", lstruKernelTime);
        PrintThreadTime("用户时间", lstruUserTime);
    }
}
//根据类型输出线程时间
void CThreadGroup::PrintThreadTime(char* apTime, FILETIME& aoFileTime)
{
    FILETIME lstruLocalTime;
    
    if(false == FileTimeToLocalFileTime(&aoFileTime, &lstruLocalTime)) {
        return;
    }
    
    SYSTEMTIME lstruSystemTime;
    
    if(false == FileTimeToSystemTime(&lstruLocalTime, &lstruSystemTime)) {
        return;
    }
    
    TRACE(1, "CThreadGroup::Dump " << apTime << " "
          << lstruSystemTime.wYear << "-" << lstruSystemTime.wMonth << "-" << lstruSystemTime.wDay << "-"
          << lstruSystemTime.wHour << "-" << lstruSystemTime.wMinute << "-" << lstruSystemTime.wSecond << "-"
          << lstruSystemTime.wMilliseconds);
}

#else
void CThreadGroup::PrintThreadInfo(STRU_THREAD_CONTEXT& aoThreadContext)
{
    ASSERT(&aoThreadContext != NULL);
    //打印线程信息
    TRACE(1, "CThreadGroup::Dump " << "线程ID:" << aoThreadContext.muiThreadID
          << " 线程名称:" << aoThreadContext.moThreadStat.GetThreadName()
          << " 性能:" << aoThreadContext.moThreadStat.GetPerformance()
          << " 优先级:" << aoThreadContext.miPriority);
    struct tm* lptm = localtime(&aoThreadContext.moStartTime.time);
    char lszTemp[256];
    sprintf(lszTemp, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
            1900 + lptm->tm_year, 1 + lptm->tm_mon, lptm->tm_mday,
            lptm->tm_hour, lptm->tm_min, lptm->tm_sec, aoThreadContext.moStartTime.millitm);
    TRACE(1, "CThreadGroup::Dump 创建时间 " << lszTemp);
    
    //当sig参数为0时，函数将进行错误检查，不发送信号，用来检查tid的合法性。
    if(0 == pthread_kill(aoThreadContext.muiThreadID, 0)) {
        TRACE(1, "CThreadGroup::Dump 线程没有退出，存活计数:"
              << aoThreadContext.moThreadStat.GetAliveCounter());
    } else {
        TRACE(1, "CThreadGroup::Dump 线程已经退出，存活计数:"
              << aoThreadContext.moThreadStat.GetAliveCounter());
    }
}
#endif

bool CThreadGroup::Detach(STRU_THREAD_CONTEXT& apThread)
{
    bool lbRet = false;
    list<STRU_THREAD_CONTEXT*>::iterator loIterator;
    
    //如果是停止状态,重新设置停止标记
    if(mbStop) {
        TRACE(1, "CThreadGroup::Detach 处于结束状态。");
        return true;
    }
    
    {
        CAutoLock loLock(moAccessCS);
        
        for(loIterator = moThreadList.begin(); loIterator != moThreadList.end(); loIterator++) {
            if((*loIterator) == &apThread) {
                lbRet = true;
                moThreadList.erase(loIterator);
                break;
            }
        }
    }
    
    if(lbRet) {
        TRACE(1, "CThreadGroup::Detach 线程分离。");
        EndThread(&apThread);
        PrintThreadInfo(apThread);
        delete(&apThread);
    }
    
    return lbRet;
}
