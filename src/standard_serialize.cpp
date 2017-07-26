/*********************************************************************************
 * author: isold.wang@gmail.com
***********************************************************************************/

#include "standard_serialize.h"

CStandardSerialize::CStandardSerialize(char* apBuffer, int alBufLen, ENUM_TYPE abyType)
{
    mpBuffer = apBuffer;
    mbyType  = abyType;
    mlBufLen = alBufLen;
    mlDataLen = 0;
}

int CStandardSerialize::getDataLen()
{
    return mlDataLen;
}

#ifndef x86_64
int CStandardSerialize::Serialize(int64_t& aiValue)
{
    if(mlBufLen < (mlDataLen + sizeof(int64_t)))
        throw(-1);
        
    if(mbyType == LOAD) {
        memcpy(&aiValue, mpBuffer + mlDataLen, sizeof(int64_t));
    } else {
        memcpy(mpBuffer + mlDataLen, &aiValue, sizeof(int64_t));
    }
    
    mlDataLen += sizeof(int64_t);
    return 1;
}
#endif

//*****************************************************************************
//  函数：  序列化以0结尾的字符串数据
//  参数：  char * apValue      字符串数据
//          uint16_t awBufferLen    容纳此字符串数据的缓存区大小
//  返回值：int  1= 成功； -1 = 失败
//  用法：
//*****************************************************************************
int CStandardSerialize::Serialize(char* apValue, uint16_t awMaxLen)
{
    if(mlBufLen < (mlDataLen + 2))
        throw(-1);
        
    uint16_t    lwLen = 0;
    
    if(mbyType == LOAD) {   //读取
        //首先读取长度
        memcpy(&lwLen, mpBuffer + mlDataLen, 2);
        mlDataLen += 2;
        
        //读取数据本身
        if((lwLen >= awMaxLen) || ((mlDataLen + lwLen) > mlBufLen)) {
            throw(-1);
        }
        
        memcpy(apValue, mpBuffer + mlDataLen, lwLen);
        apValue[lwLen] = '\0';
        mlDataLen += lwLen;
    } else { //存储
        //首先存储长度
        lwLen = strlen(apValue);
        
        if((lwLen >= awMaxLen) || (lwLen + mlDataLen + 2 > mlBufLen))
            throw(-1);
            
        memcpy(mpBuffer + mlDataLen, &lwLen, 2);
        mlDataLen += 2;
        //存储数据本身
        memcpy(mpBuffer + mlDataLen, apValue, lwLen);
        mlDataLen += lwLen;
    }
    
    return 1;
}

//*****************************************************************************
//  函数：  序列化数据
//  参数：  char * apValue      数据
//          uint16_t& awLen         此数据的真正长度
//          uint16_t awBufferLen    容纳此数据的缓存区大小
//  返回值：int  1= 成功； -1 = 失败
//  用法：
//*****************************************************************************
int CStandardSerialize::Serialize(char* apValue, uint16_t awLen, uint16_t aiBufferLen)
{
    if((awLen > aiBufferLen) || (mlBufLen < (mlDataLen + awLen))) {
        TRACE(1, "CStandardSerialize::Serialize  <awLen>：" << awLen << " <aiBufferLen>：" << aiBufferLen << "<mlBufLen>：" << mlBufLen << "<mlDataLen>：" << mlDataLen);
        throw(-1);
    }
    
    if(mbyType == LOAD) {   //读取
        //因为外部制定了读取长度，所以不需要对数据长度进行序列化
        memcpy(apValue, mpBuffer + mlDataLen, awLen);
    } else { //存储数据本身
        memcpy(mpBuffer + mlDataLen, apValue, awLen);
    }
    
    mlDataLen += awLen;
    return 1;
}
//RSA 加解密需要
int CStandardSerialize::Serialize(uint8_t* apValue, uint16_t awLen)
{
    if(mlBufLen < (mlDataLen + awLen))
        throw(-1);
        
    if(mbyType == LOAD) {   //读取
        //因为外部制定了读取长度，所以不需要对数据长度进行序列化
        memcpy(apValue, mpBuffer + mlDataLen, awLen);
    } else { //存储数据本身
        memcpy(mpBuffer + mlDataLen, apValue, awLen);
    }
    
    mlDataLen += awLen;
    return 1;
}


