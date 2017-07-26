#ifndef _THREAD_GROUP_H_
#define _THREAD_GROUP_H_

#include <list>
#include "include.h"
#include "critical_section.h"

//线程工作环境
struct STRU_THREAD_CONTEXT;

//定义使用的线程函数原型和启动线程需要的函数原型
#ifdef WIN32
#include <process.h>
typedef unsigned(__stdcall ThreadFunc)(STRU_THREAD_CONTEXT&);
typedef unsigned(__stdcall StdThreadFunc)(void*);
#else
typedef unsigned(ThreadFunc)(STRU_THREAD_CONTEXT&);
typedef void* (StdThreadFunc)(void*);
#endif

//提供线程的监视功能
class CThreadStat
{

public:
    CThreadStat()
        : miStep(0)
    {
        memset(this, 0, sizeof(CThreadStat));
        Alive();
        threadIndex = 0;
    }
    
    //汇报存活
    void Alive()
    {
        ftime(&mtLastAliveTime);
        mui64AliveTimestamp = mtLastAliveTime.time * 1000 + mtLastAliveTime.millitm;
        ++muiAliveCounter;
    }
    
    //取存活计数
    int GetAliveCounter()
    {
        int liRet = muiAliveCounter;
        muiAliveCounter = 0;
        return liRet;
    }
    
    int GetAliveCounter(int)
    {
        return muiAliveCounter;
    }
    
    struct timeb GetLastAliveTime() const {
        return mtLastAliveTime;
    }
    
    int64_t GetAliveTimestamp() const
    {
        return mui64AliveTimestamp;
    }
    
    //设置名称和编号
    void SetThreadName(char* apThreadName, int aiNum = 0)
    {
        threadIndex = aiNum;
        
        if(apThreadName) {
            sprintf(macThreadName, "%d号%s", aiNum, apThreadName);
        }
    }
    
    //取线程名称
    char* GetThreadName()
    {
        return macThreadName;
    }
    
    //取性能参数
    long GetPerformance()
    {
        long llTemp = mlPerformance;
        mlPerformance = 0;
        return llTemp;
    }
    
    //设置线程的性能参数，用于统计耗时操作执行的时间
    void SetPerformance(long alPerformance)
    {
        if(mlPerformance < alPerformance) {
            mlPerformance = alPerformance;
        }
    }
    
    void SetStep(int n)
    {
        miStep = n;
    }
    
    int GetStep() const
    {
        return miStep;
    }
    
    int GetThreadIndex()
    {
        return threadIndex;
    }
    
private:
    uint32_t    muiAliveCounter;        // 线程存活计数
    int64_t           mui64AliveTimestamp;    // 存活时间戳
    struct timeb    mtLastAliveTime;        // 最后一次存活时间
    long            mlPerformance;          // 线程性能
    char            macThreadName[64 + 1];  // 线程名称
    int             miStep;
    int threadIndex;
};

class CThreadGroup;

struct STRU_THREAD_CONTEXT {
public:
    void*           mpWorkContext;      //工作的环境
    CThreadGroup*   mpGroupManager;     //管理者
    CThreadStat     moThreadStat;       //统计
    int             miPriority;         //线程的优先级
#ifdef WIN32
    HANDLE          mhHandle;           //线程句柄
    uint32_t    muiThreadID;        //线程ID
#else
    timeb           moStartTime;        //启动时间
    pthread_t       muiThreadID;        //线程ID
#endif
};

//线程组管理对象
class CThreadGroup
{
public:
    //构造、析构
    CThreadGroup(void);
    ~CThreadGroup(void);
    
public:
    //开始线程的数量
    uint32_t Start(ThreadFunc* apThreadFucn, void* apWorkContext,
                   uint32_t auiThreadNum, char* apThreadName = NULL, int aiPriority = 0);
                   
    //是否停止
    bool IsStop()
    {
        return mbStop;
    };
    //停止的线程
    bool StopAll();
    
    //和管理者分离
    bool Detach(STRU_THREAD_CONTEXT& apThread);
    //输出调试信息
    void Dump();
    
    //可中断的等待
    bool Sleep(uint32_t ldwWaitTime);
    
    //唤醒所以线程
    void WakeAll();
    
private:
    //启动线程
    bool StartThread(STRU_THREAD_CONTEXT* apContext, ThreadFunc* apThreadFunc);
    //停止线程
    bool EndThread(STRU_THREAD_CONTEXT* apContext);
    
    //打印线程的信息
    void PrintThreadInfo(STRU_THREAD_CONTEXT& aThreadContext);
    
#ifdef WIN32
    //根据类型输出线程时间
    void PrintThreadTime(char* apTime, FILETIME& aoFileTime);
#endif
    
private:
    //停止标记
    bool                        mbStop;
    //线程信息列表
    std::list<STRU_THREAD_CONTEXT*>  moThreadList;
    //等待使用的句柄
    CEvent                      moStopEvent;
    //线程信息的保护
    CCriticalSection            moAccessCS;
};
#endif //_THREAD_GROUP_H_
