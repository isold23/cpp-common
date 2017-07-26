#include "include.h"
#include "critical_section.h"

CCriticalSection::CCriticalSection()
{
#ifdef WIN32
    InitializeCriticalSection(&moSection);
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mMutex, &attr);
#endif
}

CCriticalSection::~CCriticalSection()
{
#ifdef WIN32
    DeleteCriticalSection(&moSection);
#else
    pthread_mutex_destroy(&mMutex);
#endif
}

CAutoLock::CAutoLock(CCriticalSection& aoSection): moSection(aoSection)
{
    moSection.Enter();
}

CAutoLock::~CAutoLock()
{
    moSection.Leave();
}

CEvent::CEvent(const char* apName)
    : mbCreated(false)
{
    mstrName = apName;
#ifdef WIN32
    mhEventHandle = NULL;
#else
    //mhCond_t = PTHREAD_COND_INITIALIZER;
#endif
}

CEvent::~CEvent()
{
#ifdef WIN32
    ASSERT(mhEventHandle == NULL);
#else
#endif
}

bool CEvent::Create(bool bManualReset /* = false */, bool bInitialState /* = false */)
{
#ifdef WIN32
    mhEventHandle = CreateEvent(NULL, abManualReset, abInitialState, NULL);
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutex_init(&mhMutex, &attr);
    pthread_mutexattr_destroy(&attr);
    pthread_cond_init(&mhCond_t, NULL);
#endif
    mbCreated = true;
    return true;
}

int CEvent::WaitForEvent(uint32_t dwMilliseconds)
{
#ifdef WIN32
    uint32_t ldwResult = WaitForSingleObject(mhEventHandle, dwMilliseconds);
    
    if(ldwResult == WAIT_OBJECT_0) {
        return 0;
    } else if(ldwResult == WAIT_TIMEOUT) {
        return -1;
    }
    
    return -2;
#else
    
    if(dwMilliseconds == (uint32_t) - 1) {
        //使线程阻塞在一个条件变量的互斥锁上，无条件等待
        pthread_mutex_lock(&mhMutex);
        pthread_cond_wait(&mhCond_t, &mhMutex);
        pthread_mutex_unlock(&mhMutex);
        return 0;
    }
    
    struct timeval now;      /*time when we started waiting*/
    
    struct timespec timeout; /*timeout value for the wait function */
    
    pthread_mutex_lock(&mhMutex);       //Lock
    
    gettimeofday(&now, NULL);
    
    timeout.tv_sec = now.tv_sec + dwMilliseconds / 1000;
    
    timeout.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000) * 1000;
    
    int ldwResult = pthread_cond_timedwait(&mhCond_t, &mhMutex, &timeout);
    
    pthread_mutex_unlock(&mhMutex);     //UnLock
    
    if(ldwResult == ETIMEDOUT) {
        return -1;
    }
    
    return 0;
#endif
}

void CEvent::SetEvent()
{
#ifdef WIN32

    if(mhEventHandle) {
        ::SetEvent(mhEventHandle);
    }
    
#else
    pthread_mutex_lock(&mhMutex);
    pthread_cond_broadcast(&mhCond_t);
    pthread_mutex_unlock(&mhMutex);
#endif
}

//重新设置事件为无信号
void CEvent::ResetEvent()
{
    //TRACE(5,"CEvent::ResetEvent, ..."<<",name:"<<mstrName.c_str());
#ifdef WIN32
    if(mhEventHandle) {
        ::ResetEvent(mhEventHandle);
    }
    
#endif
}

//关闭事件
void CEvent::Close()
{
    //TRACE(5,"CEvent::Close, ..."<<",name:"<<mstrName.c_str());
    if(mbCreated) {
#ifdef WIN32
    
        if(mhEventHandle != NULL) {
            CloseHandle(mhEventHandle);
            mhEventHandle = NULL;
        }
        
#else
        pthread_cond_destroy(&mhCond_t);
        pthread_mutex_destroy(&mhMutex);
#endif
        mbCreated = false;
    }
}


#ifndef WIN32
Conditional::Conditional(const char* id)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutex_init(&_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    pthread_cond_init(&_cond, NULL);
    miDataCount = 0;
}

Conditional::~Conditional()
{
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
    miDataCount = 0;
}

bool Conditional::tryEnterMutex(void)
{
    if(pthread_mutex_trylock(&_mutex) != 0)
        return false;
        
    return true;
}

void Conditional::enterMutex(void)
{
    pthread_mutex_lock(&_mutex);
}

void Conditional::leaveMutex(void)
{
    pthread_mutex_unlock(&_mutex);
}

void Conditional::signal(bool broadcast)
{
    enterMutex();
    miDataCount++;
    
    if(broadcast)
        pthread_cond_broadcast(&_cond);
    else
        pthread_cond_signal(&_cond);
        
    leaveMutex();
}

bool Conditional::wait(timeout_t timeout, bool locked)
{
    struct timespec ts;
    int rc;
    
    if(!locked)
        enterMutex();
        
    if(!timeout) {
        if(miDataCount-- <= 0) {
            pthread_cond_wait(&_cond, &_mutex);
        }
        
        if(!locked)
            leaveMutex();
            
        return true;
    }
    
    getTimeoutEx(&ts, timeout);
    rc = pthread_cond_timedwait(&_cond, &_mutex, &ts);
    
    if(!locked)
        leaveMutex();
        
    if(rc == ETIMEDOUT)
        return false;
        
    return true;
}

#endif

#ifdef WIN32

CProcessMutex::CProcessMutex(const char* name)
{
    memset(m_cMutexName, 0 , sizeof(m_cMutexName));
    int min = strlen(name) > (sizeof(m_cMutexName) - 1) ? (sizeof(m_cMutexName) - 1) : strlen(name);
    strncpy(m_cMutexName, name, min);
    m_pMutex = CreateMutex(NULL, false, m_cMutexName);
}

CProcessMutex::~CProcessMutex()
{
    CloseHandle(m_pMutex);
}

bool CProcessMutex::Lock()
{
    if(NULL == m_pMutex) {
        return false;
    }
    
    DWORD nRet = WaitForSingleObject(m_pMutex, INFINITE);
    
    if(nRet != WAIT_OBJECT_0) {
        return false;
    }
    
    return true;
}

bool CProcessMutex::UnLock()
{
    return ReleaseMutex(m_pMutex);
}

#endif

#ifndef WIN32

CProcessMutex::CProcessMutex(const char* name)
{
    if(name != NULL) {
        memset(m_cMutexName, 0 , sizeof(m_cMutexName));
        int min = strlen(name) > (sizeof(m_cMutexName) - 1) ? (sizeof(m_cMutexName) - 1) : strlen(name);
        strncpy(m_cMutexName, name, min);
    }
}

CProcessMutex::~CProcessMutex()
{
    int ret = sem_close(m_pSem);
    
    if(0 != ret) {
        printf("sem_close error %d\n", ret);
    }
    
    sem_unlink(m_cMutexName);
}

bool CProcessMutex::open(const char* name)
{
    if(name != NULL) {
        memset(m_cMutexName, 0 , sizeof(m_cMutexName));
        int min = strlen(name) > (sizeof(m_cMutexName) - 1) ? (sizeof(m_cMutexName) - 1) : strlen(name);
        strncpy(m_cMutexName, name, min);
    }
    
    m_pSem = sem_open(m_cMutexName, O_RDWR | O_CREAT, 0644, 1);
    
    if(m_pSem == SEM_FAILED) {
        return false;
    }
    
    return true;
}

bool CProcessMutex::Lock()
{
    int ret = sem_wait(m_pSem);
    
    if(ret != 0) {
        return false;
    }
    
    return true;
}

bool CProcessMutex::UnLock()
{
    int ret = sem_post(m_pSem);
    
    if(ret != 0) {
        return false;
    }
    
    return true;
}

#endif

