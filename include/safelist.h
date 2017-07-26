#ifndef DEF_SAFE_LIST_H
#define DEF_SAFE_LIST_H

#include "critical_section.h"

template <class T>
class CSafeList
{
    struct STURCT_NODE {
        T* mpData;
        STURCT_NODE* mpNext;
    };
    
public:
    inline CSafeList()
    {
        miCount = 0;
        mpHead = NULL;
        mpTail = NULL;
    }
    inline ~CSafeList()
    {
        STURCT_NODE* lpNode = NULL;
        
        while(mpHead != NULL) {
            lpNode = mpHead->mpNext;
            delete mpHead->mpData;
            delete mpHead;
            mpHead = lpNode;
        }
        
        miCount = 0;
    };
    
    inline int AddTail(T* apValue)
    {
        STURCT_NODE* lpNode = new STURCT_NODE;
        
        if(NULL == lpNode) {
            return -1;
        }
        
        lpNode->mpNext = NULL;
        lpNode->mpData = apValue;
        CAutoLock loLock(moSection);
        
        if(mpTail == NULL) {
            //没有数据
            mpHead = lpNode;
            mpTail = lpNode;
        } else {
            mpTail->mpNext = lpNode;
            mpTail = lpNode;
        }
        
        miCount++;
        return miCount;
    };
    
    inline T* DelHead()
    {
        T* lstru;
        CAutoLock loLock(moSection);
        
        if(mpHead == NULL) {
            lstru = NULL;
        } else {
            lstru = mpHead->mpData;
            STURCT_NODE* lpNode = NULL;
            lpNode = mpHead->mpNext;
            delete mpHead;
            mpHead = lpNode;
            
            if(mpHead == NULL) {
                mpTail = NULL;
            }
            
            miCount--;
        }
        
        return lstru;
    };
    
    inline void ClearAll()
    {
        STURCT_NODE* lpNode = NULL;
        CAutoLock loLock(moSection);
        
        while(mpHead != NULL) {
            lpNode = mpHead->mpNext;
            delete mpHead->mpData;
            delete mpHead;
            mpHead = lpNode;
            
            if(mpHead == NULL) {
                mpTail = NULL;
            }
        }
        
        miCount = 0;
    }
    
    inline int GetCount()
    {
        CAutoLock loLock(moSection);
        return miCount;
    };
    
    inline bool IsEmpty()
    {
        CAutoLock loLock(moSection);
        return (miCount == 0);
    }
    
private:
    int                     miCount;
    CCriticalSection        moSection;
    STURCT_NODE*             mpHead;
    STURCT_NODE*             mpTail;
};

class CSafeIntList
{
    struct STURCT_NODE {
        int miData;
        STURCT_NODE* mpNext;
    };
public:
    inline CSafeIntList()
    {
        miCount = 0;
        mpHead = NULL;
        mpTail = NULL;
    }
    inline ~CSafeIntList()
    {
        STURCT_NODE* lpNode = NULL;
        
        while(mpHead != NULL) {
            lpNode = mpHead->mpNext;
            delete mpHead;
            mpHead = lpNode;
        }
        
        miCount = 0;
    };
    
    inline int AddTail(int  aiValue)
    {
        STURCT_NODE* lpNode = new STURCT_NODE;
        
        if(NULL == lpNode) {
            return -1;
        }
        
        lpNode->mpNext = NULL;
        lpNode->miData = aiValue;
        CAutoLock loLock(moSection);
        
        if(mpTail == NULL) {
            //没有数据
            mpHead = lpNode;
            mpTail = lpNode;
        } else {
            mpTail->mpNext = lpNode;
            mpTail = lpNode;
        }
        
        miCount++;
        return miCount;
    };
    
    inline int DelHead()
    {
        int liValue = -1;
        CAutoLock loLock(moSection);
        
        if(NULL != mpHead) {
            liValue = mpHead->miData;
            STURCT_NODE* lpNode = NULL;
            lpNode = mpHead->mpNext;
            delete mpHead;
            mpHead = lpNode;
            
            if(NULL == mpHead) {
                mpTail = NULL;
            }
            
            miCount--;
        }
        
        return liValue;
    };
    
    inline void ClearAll()
    {
        STURCT_NODE* lpNode = NULL;
        CAutoLock loLock(moSection);
        
        while(mpHead != NULL) {
            lpNode = mpHead->mpNext;
            delete mpHead;
            mpHead = lpNode;
            
            if(mpHead == NULL) {
                mpTail = NULL;
            }
        }
        
        miCount = 0;
    }
    
    inline int GetCount()
    {
        CAutoLock loLock(moSection);
        return miCount;
    };
    inline bool IsEmpty()
    {
        CAutoLock loLock(moSection);
        return (miCount == 0);
    }
    
private:
    int                         miCount;
    CCriticalSection            moSection;
    STURCT_NODE*                 mpHead;
    STURCT_NODE*                 mpTail;
};

#endif //DEF_SAFE_LIST_H

