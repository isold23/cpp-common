/*********************************************************************************
file name:    StandardSerialize.h
**********************************************************************************/

#ifndef _DEF_STANDARD_SERIALIZE_H_
#define _DEF_STANDARD_SERIALIZE_H_

#include "include.h"

class CStandardSerialize
{
public:
    enum ENUM_TYPE {LOAD, STORE};
    
public:
    CStandardSerialize(char* apBuffer, int alBufLen, ENUM_TYPE abyType);
    ~CStandardSerialize() {}
    
    template<class T>
    inline int Serialize(T& value)
    {
        if(mlBufLen < (mlDataLen + (int)sizeof(T)))
            throw(-1);
            
        if(mbyType == LOAD) {
            memcpy(&value, mpBuffer + mlDataLen, sizeof(T));
        } else {
            memcpy(mpBuffer + mlDataLen, &value, sizeof(T));
        }
        
        mlDataLen += sizeof(T);
        return 1;
    }
    
#ifndef x86_64
    int Serialize(int64_t& ai64Value);
#endif
    int Serialize(char* apValue, uint16_t awMaxLen);
    int Serialize(char* apValue, uint16_t awLen, uint16_t aiBufferLen);
    int Serialize(uint8_t* apValue, uint16_t awLen);
    
    int getDataLen();
    
    //serialize type
    ENUM_TYPE mbyType;
private:
    //data length
    uint32_t mlDataLen;
    char* mpBuffer;
    // buffer length
    uint32_t mlBufLen;
};

#endif //_DEF_STANDARD_SERIALIZE_H_
