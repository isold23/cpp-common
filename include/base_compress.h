#ifndef _BASE_COMPRESS_H_
#define _BASE_COMPRESS_H_

CBaseCompress {
public:
    CBaseCompress() {}
    virtual ~CBaseCompress() {}
    
    virtual bool Compress(const char* in_buffer, uint32 in_length, char* out_buffer, uint32 & out_length) = 0;
    virtual bool Decompress(const char* in_buffer, uint32 in_length, char* out_buffer, uint32 & out_length) = 0;
    
};

#endif //_BASE_COMPRESS_H_
