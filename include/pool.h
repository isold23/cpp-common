#ifndef _DEF_POOL_H_
#define _DEF_POOL_H_

#include "include.h"
#include "safelist.h"

template<class PooledType>
class CPool
{
public:
    CPool(int alMaxPooled = 2000);
    ~CPool();
    //从缓冲池中分配数据
    PooledType* Malloc();
    //释放到缓冲区中
    void Free(PooledType* apObjet);
    //输出调试信息
    void Dump();
private:
    CSafeList<PooledType> moPool;
    CCriticalSection moAccessCS;
    int mlCreateCount;
    int mlMaxPoolSize;
};

//从缓冲池中分配数据
template< class PooledType >
PooledType* CPool<PooledType>::Malloc()
{
    PooledType* lpRetObj = NULL;
    
    //从缓冲区中取
    if(moPool.GetCount()) {
        lpRetObj = moPool.DelHead();
    }
    
    if(lpRetObj) {
        return lpRetObj;
    } else {
        //如果得不到去创建
        lpRetObj = new(std::nothrow) PooledType;
        
        if(lpRetObj) {
            CAutoLock loLock(moAccessCS);
            mlCreateCount++;
        }
        
        return lpRetObj;
    }
}

//从缓冲池中分配数据
template< class PooledType >
void CPool<PooledType>::Free(PooledType* apObjet)
{
    ASSERT(apObjet);
    
    if(apObjet) {
        if(moPool.GetCount() < mlMaxPoolSize) {
            //归还到池中
            moPool.AddTail(apObjet);
        } else {
            //直接删除
            delete apObjet;
            CAutoLock loLock(moAccessCS);
            mlCreateCount--;
        }
    }
}

//从缓冲池中分配数据
template< class PooledType >
CPool<PooledType>::CPool(int alMaxPoolSize)
{
    ASSERT(alMaxPoolSize > 0);
    mlCreateCount = 0;
    mlMaxPoolSize = alMaxPoolSize;
}

//析构
template< class PooledType >
CPool<PooledType>::~CPool()
{
    moPool.ClearAll();
}

//输出调试信息
template< class PooledType >
void CPool<PooledType>::Dump()
{
    TRACE(1, "CPool::Dump 创建数量:" << mlCreateCount << ", 最大缓冲数量:" << mlMaxPoolSize
          << ",当前空闲单元数量:" << moPool.GetCount());
}
#endif //_DEF_POOL_H_
