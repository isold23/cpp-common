#ifndef _CRITICAL_SECTION_H_
#define _CRITICAL_SECTION_H_

#ifdef WIN32

#else
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <memory.h>
#endif
#include "define.h"
#include <stdint.h>
#include <string>

typedef unsigned long   timeout_t;

class CCriticalSection
{
public:
    CCriticalSection();
    ~CCriticalSection();
    
    inline void Enter()
    {
#ifdef WIN32
        EnterCriticalSection(&moSection);
#else
        pthread_mutex_lock(&mMutex);
#endif
    }
    inline void Leave()
    {
#ifdef WIN32
        LeaveCriticalSection(&moSection);
#else
        pthread_mutex_unlock(&mMutex);
#endif
    }
    
public:
#ifdef WIN32
    CRITICAL_SECTION moSection;
#else
    pthread_mutex_t mMutex;
#endif
};

class CAutoLock
{
public:
    CAutoLock(CCriticalSection& aoSection);
    ~CAutoLock();
private:
    CCriticalSection& moSection;
};

class CEvent
{
public:
    CEvent(const char* apName = "");
    ~CEvent();
    bool Create(bool bManualReset = false, bool bInitialState = false);
    int WaitForEvent(uint32_t dwMilliseconds);
    void SetEvent();
    void ResetEvent();
    void Close();
    
private:
#ifdef WIN32
    void*               mhEventHandle;
#else
    pthread_mutex_t     mhMutex;
    pthread_cond_t      mhCond_t;
#endif
    std::string  mstrName;
    bool    mbCreated;
};


#ifndef WIN32
/**
* A conditional variable synchcronization object for one to one and
* one to many signal and control events between processes.
* Conditional variables may wait for and receive signals to notify
* when to resume or perform operations.  Multiple waiting threads may
* be woken with a broadcast signal.
*
* @warning While this class inherits from Mutex, the methods of the
* class Conditional just handle the system conditional variable, so
* the user is responsible for calling enterMutex and leaveMutex so as
* to avoid race conditions. Another thing to note is that if you have
* several threads waiting on one condition, not uncommon in thread
* pools, each thread must take care to manually unlock the mutex if
* cancellation occurs. Otherwise the first thread cancelled will
* deadlock the rest of the thread.
*
* @author David Sugar
* @short conditional.
* @todo implement in win32
*/
class  Conditional
{
private:
    pthread_cond_t _cond;
    pthread_mutex_t _mutex;
    int miDataCount;
    
public:
    /**
    * Create an instance of a conditional.
    *
    * @param id name of conditional, optional for deadlock testing.
    */
    Conditional(const char* id = NULL);
    
    /**
    * Destroy the conditional.
    */
    virtual ~Conditional();
    
    /**
    * Signal a conditional object and a waiting threads.
    *
    * @param broadcast this signal to all waiting threads if true.
    */
    void signal(bool broadcast);
    
    /**
    * Wait to be signaled from another thread.
    *
    * @param timer time period to wait.
    * @param locked flag if already locked the mutex.
    */
    bool wait(timeout_t timer = 0, bool locked = false);
    
    /**
    * Locks the conditional's mutex for this thread.  Remember
    * that Conditional's mutex is NOT a recursive mutex!
    *
    * @see #leaveMutex
    */
    void enterMutex(void);
    
    /**
    * In the future we will use lock in place of enterMutex since
    * the conditional composite is not a recursive mutex, and hence
    * using enterMutex may cause confusion in expectation with the
    * behavior of the Mutex class.
    *
    * @see #enterMutex
    */
    inline void lock(void)
    {
        enterMutex();
    };
    
    /**
    * Tries to lock the conditional for the current thread.
    * Behaves like #enterMutex , except that it doesn't block the
    * calling thread.
    *
    * @return true if locking the mutex was succesful otherwise false
    *
    * @see enterMutex
    * @see leaveMutex
    */
    bool tryEnterMutex(void);
    
    inline bool test(void)
    {
        return tryEnterMutex();
    };
    
    /**
    * Leaving a mutex frees that mutex for use by another thread.
    *
    * @see #enterMutex
    */
    void leaveMutex(void);
    
    inline void unlock(void)
    {
        return leaveMutex();
    };
    
    timespec* getTimeoutEx(struct timespec* spec, timeout_t timer)
    {
        static  struct timespec myspec;
        
        if(spec == NULL)
            spec = &myspec;
            
#ifdef  PTHREAD_GET_EXPIRATION_NP
        struct timespec offset;
        offset.tv_sec = timer / 1000;
        offset.tv_nsec = (timer % 1000) * 1000000;
        pthread_get_expiration_np(&offset, spec);
#else
        struct timeval current;
        gettimeofday(&current, NULL);
        spec->tv_sec = current.tv_sec + ((timer + current.tv_usec / 1000) / 1000);
        spec->tv_nsec = ((current.tv_usec / 1000 + timer) % 1000) * 1000000;
#endif
        return spec;
    }
};
#endif

class CProcessMutex
{
public:
    CProcessMutex(const char* name = NULL);
    ~CProcessMutex();
    
    bool open(const char* name = NULL);
    
    bool Lock();
    bool UnLock();
    
private:

#ifdef WIN32
    void* m_pMutex;
#endif
    
#ifdef linux
    sem_t* m_pSem;
#endif
    char m_cMutexName[100];
};



#endif //_CRITICAL_SECTION_H_
