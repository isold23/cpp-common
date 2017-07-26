#ifndef COMMON_TWO_LOCK_QUEUE_H_
#define COMMON_TWO_LOCK_QUEUE_H_

/**********************************************************************
author: isold.wang@gmail.com
description:
        1 基于stl 的list实现
        2 这个队列是先入先出队列(尾插头出)
        3 有2把锁,类似读写锁,只是一个锁队列头,一个锁队列尾
        4 据文档上描述在32进程操作时比一把锁操作queue性能提高10% 左右

***********************************************************************/
#include <pthread.h>

template <typename T>
class TwoLockQueue
{
public:
    TwoLockQueue()
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&HMutex, &attr);
        pthread_mutex_init(&TMutex, &attr);
        //初始化list,加入一个哨兵node,让头和尾指针都指向它
        list.push_back(T());
        iHead = list.begin();
        iTail = list.end();
    }
    ~TwoLockQueue()
    {
        pthread_mutex_lock(&HMutex);
        list.erase(iHead);
        pthread_mutex_unlock(&HMutex);
        pthread_mutex_destroy(&HMutex);
        pthread_mutex_destroy(&TMutex);
    }
    
    void push(const T& t)
    {
        try {
            //加尾锁
            pthread_mutex_lock(&TMutex);
            //追加元素
            list.push_back(t);
            //获取list 尾迭代器
            iTail = list.end();
            //解尾锁
            pthread_mutex_unlock(&TMutex);
        } catch(...) {
            //解尾锁
            pthread_mutex_unlock(&TMutex);
        }
    }
    
    bool pop(T& t)
    {
        //加头锁
        pthread_mutex_lock(&HMutex);
        //获取list头迭代器
        typename TList::iterator iNext = iHead;
        //获取要弹出内容的迭代器
        ++iNext;
        
        if(iNext != list.end()) {  //判断queue是否为空
            //获取要弹出内容
            t = *iNext;
            //释放无用的内容
            list.erase(iNext);
            //解头锁
            pthread_mutex_unlock(&HMutex);
            return true;
        }
        
        //解头锁
        pthread_mutex_unlock(&HMutex);
        return false;
    }
    
    int size()
    {
        int count = 0;
        //加头锁
        pthread_mutex_lock(&HMutex);
        {
            //加尾锁
            pthread_mutex_lock(&TMutex);
            //取list长度
            count = list.size();
            //去掉头结点的基数
            count--;
            //解头锁
            pthread_mutex_unlock(&HMutex);
        }
        //解尾锁
        pthread_mutex_unlock(&TMutex);
        return count;
    }
    
    bool empty()
    {
        //加尾锁
        pthread_mutex_lock(&TMutex);
        
        if(iHead != iTail) {  //判断queue是否为空,(不为空)
            //解头锁
            pthread_mutex_unlock(&TMutex);
            return false;
        }
        
        //解尾锁
        pthread_mutex_unlock(&TMutex);
        return true;
    }
private:
    typedef std::list<T> TList;
    TList list;
    pthread_mutex_t HMutex;
    // stl在模板里面嵌套的时候需要typename 标识
    typename TList::iterator iHead;
    pthread_mutex_t TMutex;
    // stl在模板里面嵌套的时候需要typename 标识
    typename TList::iterator iTail;
};

#endif

