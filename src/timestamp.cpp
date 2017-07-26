#include "timestamp.h"

CTimeStamp::CTimeStamp()
    : moRecordPool(200000)
{
    for(int i = 0; i < RECORD_MAX_COUNT; i++) {
        mpList[i] = NULL;
    }
    
    mdwRepeatCount = 0;
}

CTimeStamp::~CTimeStamp()
{
    int i = 0;
    CRecordItem* lpNext = NULL;
    CRecordItem* lpRecordItem = NULL;
    CAutoLock loLock(mCriSection);
    
    for(i = 0; i < RECORD_MAX_COUNT; i++) {
        if(mpList[i] != NULL) {
            lpNext = mpList[i];
            lpRecordItem = mpList[i];
            
            while(lpNext != NULL) {
                lpRecordItem = lpNext;
                lpNext = lpRecordItem->mpNext;
                delete lpRecordItem;
                lpRecordItem = NULL;
            }
            
            mpList[i] = NULL;
        }
    }
}

bool CTimeStamp::CheckTimeStamp(uint16_t awPackSerial, uint32_t aulIpAddr, uint16_t awPort)
{
    bool  lbResult = false;
    time_t  ltmNow;
    uint16_t lwIndex = awPackSerial;//除8
    CRecordItem* lpNext = NULL;
    CRecordItem* lpRecordItem = NULL;
    int llTime = 0;
    CAutoLock loLock(mCriSection);
    time(&ltmNow);
    llTime = ltmNow - TIME_STAMP_SECOND;
    lpNext = mpList[lwIndex];
//#define UCASM 1
#ifndef UCASM

    while(lpNext != NULL) {
        lpRecordItem = lpNext;
        lpNext = lpRecordItem->mpNext;
        
        if(llTime > lpRecordItem->mlTime) {
            if(lpRecordItem->mpPre == NULL) {
                //最前一个
                mpList[lwIndex] = lpNext;
                
                if(lpNext != NULL) {
                    lpNext->mpPre = NULL;
                }
            } else {
                lpRecordItem->mpPre->mpNext = lpNext;
                
                if(lpNext != NULL) {
                    lpNext->mpPre = lpRecordItem->mpPre;
                }
            }
            
            moRecordPool.Free(lpRecordItem);
            lpRecordItem = NULL;
        } else {
            if(lpRecordItem->mulIpAddr == aulIpAddr &&  lpRecordItem->mwPort == awPort) {
                lbResult = true;
                lpRecordItem->mlTime = ltmNow;
                mdwRepeatCount++;
            }
        }
    }
    
    if(lbResult == false) {
        lpNext = moRecordPool.Malloc();
        lpNext->mulIpAddr = aulIpAddr;
        lpNext->mwPort = awPort;
        lpNext->mlTime = ltmNow;
        lpNext->mpNext  = NULL;
        lpNext->mpPre   = NULL;
        
        if(lpRecordItem != NULL) {
            lpRecordItem->mpNext = lpNext;
            lpNext->mpPre = lpRecordItem;
        } else {
            mpList[lwIndex] = lpNext;
        }
    }
    
#else
    __asm {
        //mov edx,uint32_t ptr [lpNext];
        mov edx, lpNext;
        WHILE:
        cmp edx, 0; //while (lpNext != NULL)
        jz END;
        mov ebx, edx; //lpRecordItem =lpNext;
        mov edx, dword ptr [ebx+0Ch] ; //lpNext = lpRecordItem->mpNext;
        //mov dword ptr [lpNext],edx;
        mov ecx, uint32_t ptr[llTime]; //if (llTime>lpRecordItem->mlTime  )
        cmp ecx, dword ptr [ebx+8] ;
        jle ELSE1;
        cmp dword ptr [ebx+10h], 0; //if (lpRecordItem->mpPre == NULL)
        jne ELSE2;
        //}
        //mpList[lwIndex]=lpNext;
        //__asm
        //{
        movzx       eax, word ptr [lwIndex] ; //    mpList[lwIndex]=lpNext;
        mov         ecx, dword ptr [this] ;
        mov         dword ptr [ecx+eax*4], edx ;
        //mov edx,dword  ptr [lpNext];
        cmp edx, 0; //if (lpNext != NULL)
        je GOTO1;
        mov dword ptr [edx+0Ch], 0; //lpNext->mpPre =NULL;
        jmp GOTO1;
        ELSE2://else if(lptempInfo == lpoHeadNode)
        mov eax, dword ptr [ebx+10h] ; //lpRecordItem->mpPre->mpNext =lpNext;
        mov dword ptr [eax+0Ch], edx
        cmp edx, 0; //  if (lpNext != NULL)
        je  GOTO1;
        mov eax, dword ptr [ebx+10h]; //    lpNext->mpPre =lpRecordItem->mpPre;
        mov dword ptr [edx+10h], eax
        GOTO1:
    }
    delete lpRecordItem;
    lpRecordItem = NULL;
    __asm {
        jmp WHILE;
        ELSE1://else if (llTime>lpRecordItem->mlTime  )
        mov eax, dword ptr [ebx] //if (lpRecordItem->mulIpAddr == aulIpAddr &&
        cmp eax, dword ptr [aulIpAddr];
        jne END1;
        movzx eax, word ptr [ebx+4]; //lpRecordItem->mwPort == awPort)
        movzx ecx, word ptr [awPort];
        cmp eax, ecx;
        jne END1;
        mov byte ptr [lbResult], 1 ; //lbResult = true;
        mov eax, dword ptr [ltmNow]; //lpRecordItem->mlTime = ltmNow;
        mov dword ptr[ebx+8], eax;
        END1:
        jmp WHILE;//}
        END:
        movzx eax, byte ptr [lbResult] ; //if (lbResult == false)
        test        eax, eax    ;
        jne   ENDNEW;
    }
    lpNext = new CRecordItem;
    __asm {
        mov edx, dword ptr [lpNext]; //lpNext->mulIpAddr = aulIpAddr;
        mov eax, dword ptr [aulIpAddr];
        mov dword ptr [edx], eax ;
        mov cx, word ptr [awPort]; //lpNext->mwPort = awPort;
        mov word ptr [edx+4], cx ;
        mov eax, dword ptr [ltmNow]; //lpNext->mlTime = ltmNow;
        mov dword ptr [edx+8], eax ;
        mov dword ptr [edx+0Ch], 0 ; //lpNext->mpNext  = NULL;
        mov dword ptr [edx+10h], 0 ; //lpNext->mpPre   = NULL;
        mov ebx, dword ptr [lpRecordItem]; //if (lpRecordItem != NULL)
        cmp ebx, 0;
        je ELSENEW;
        mov dword ptr [ebx+0Ch], edx ; //lpRecordItem->mpNext =lpNext;
        mov dword ptr [edx+10h], ebx ; //lpNext->mpPre = lpRecordItem;
        jmp ENDNEW;
        ELSENEW:
        movzx       eax, word ptr [lwIndex] ; //    mpList[lwIndex]=lpNext;
        mov         ecx, dword ptr [this] ;
        mov         dword ptr [ecx+eax*4], edx ;
        ENDNEW:
        ;
    }
#endif
    return lbResult;
}

void CTimeStamp::Dump()
{
    TRACE(1, "CTimeStamp::Dump 当前时间戳检查未过期个数:" << mdwRepeatCount);
    moRecordPool.Dump();
}
