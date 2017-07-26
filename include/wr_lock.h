#ifndef _AUTO_WR_LOCK_H
#define _AUTO_WR_LOCK_H (1)
/*
  *file AutoWRlock.h
  *brief linux read and write autolock class
  *author isold.wang@gmail.com
  *version 0.1
  *date 2010.11.26
  */
/*******************************************************************************
 * include
 *******************************************************************************/
#ifdef WIN32

#else
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#endif
#include "define.h"
#include <string>

/*******************************************************************************
 * class/struct
 *******************************************************************************/
class CRWSection
{
public:
    CRWSection();
    ~CRWSection();
    //进入写临界区
    inline void EnterWrite()
    {
#ifdef WIN32
#else
        pthread_rwlock_wrlock(&mrwlock);
#endif
    }
    //进入读临界区
    inline void EnterRead()
    {
#ifdef WIN32
#else
        pthread_rwlock_rdlock(&mrwlock);
#endif
    }
    //离开读写临界区
    inline void LeaveWR()
    {
#ifdef WIN32
#else
        pthread_rwlock_unlock(&mrwlock);
#endif
    }
public:
#ifdef WIN32

#else
    //Linux平台读写锁对象
    pthread_rwlock_t mrwlock;
#endif
};

//自动写锁实现类
class CAutoWRLock
{
public:
    //构造函数
    CAutoWRLock(CRWSection& aSection);
    //析构函数
    ~CAutoWRLock();
private:
    CRWSection& mSection;
};
//自动读锁实现类
class CAutoRDLock
{
public:
    //构造函数
    CAutoRDLock(CRWSection& aSection);
    //析构函数
    ~CAutoRDLock();
private:
    CRWSection& mSection;
};

#endif

