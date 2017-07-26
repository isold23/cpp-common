#ifndef DEF_TIME_STAMP_H
#define DEF_TIME_STAMP_H

#include "include.h"
#include "pool.h"

#define RECORD_MAX_COUNT        65536
#define TIME_STAMP_SECOND       7   //7秒钟超时

//时间戳信息结构
class CRecordItem
{
public:
    uint32_t    mulIpAddr;  //IP地址
    uint16_t          mwPort;     //端口
    int         mlTime;     //时间
    CRecordItem* mpNext;
    CRecordItem* mpPre;
};

//时间戳类
class CTimeStamp
{
private:
    CRecordItem*         mpList[RECORD_MAX_COUNT];  //时间戳保存数组
    CCriticalSection    mCriSection;                //保护数组的临界区
    
    CPool<CRecordItem>  moRecordPool;               //时间戳缓冲池
    
    uint32_t                mdwRepeatCount;             //时间戳检查未过期的次数
    
public:
    CTimeStamp();
    ~CTimeStamp();
    
    //检查时间戳是否过期
    bool CheckTimeStamp(uint16_t awPackSerial, uint32_t aulIpAddr, uint16_t awPort);
    
    //运行数据输出
    void Dump();
};

#endif
