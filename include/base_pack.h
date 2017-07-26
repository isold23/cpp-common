#ifndef _BASE_PACK_H_
#define _BASE_PACK_H_

#include "include.h"

class CBasePack
{
public:
    CBasePack(uint16 PackType = 0)
        : mui16PackType(PackType)
    {}
    virtual ~CBasePack(void) {}
    //pack , >0 suc, <=0 error
    int Pack(char* apBuff, uint32 aiBuffSize);
    //unpack,0 suc, <0 error
    int UnPack(const char* apData, uint32 ausDataLen);
    //return packet type
    inline uint16 GetPackType(void)
    {
        return mui16PackType;
    }
    inline void SetPackType(uint16 type)
    {
        mui16PackType = type;
    }
    
protected:
    //serialize data members
    virtual int Serialize(CStandardSerialize& aoSerialize) = 0;
protected:
    uint16 mui16PackType;
};

//pack , >0 suc, <=0 error
inline int CBasePack::Pack(char* apBuff, uint32 aiBuffSize)
{
    try {
        CStandardSerialize loSerialize(apBuff, aiBuffSize, CStandardSerialize::STORE);
        memset(apBuff, 0, aiBuffSize);
        loSerialize.Serialize(mui16PackType);
        
        if(Serialize(loSerialize) == -1)
            return -1;
            
        return loSerialize.getDataLen();
    } catch(...) {
        return -1;
    }
}
//unpack,0 suc, <0 error
inline int CBasePack::UnPack(const char* apData, uint32 ausDataLen)
{
    try {
        CStandardSerialize loSerialize((char*)apData, ausDataLen, CStandardSerialize::LOAD);
        uint16 lwPackType ;
        loSerialize.Serialize(lwPackType);
        
        if(lwPackType != mui16PackType)
            return -1;
            
        if(Serialize(loSerialize) == -1)
            return -1;
            
        return 0;
    } catch(...) {
        return -1;
    }
}

#endif //_BASE_PACK_H_



