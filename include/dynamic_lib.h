/***********************************************************************
 * author: isold.wang@gmail.com
***********************************************************************/

#ifndef DEF_DYNAMIC_LINK_LIBRARY_H
#define DEF_DYNAMIC_LINK_LIBRARY_H

#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include "include.h"

class CDynamicLib
{
public:
    CDynamicLib(void);
    ~CDynamicLib(void);
    bool LoadDynamicLib(const char* apFileName);
    void* GetDynamicLibFun(const char* apFunName);
    void CloseDynamicLib();
    
private:
#ifdef WIN32
    HMODULE mhDynamic;
#else
    void* mhDynamic;
#endif
};

#endif //DEF_DYNAMIC_LINK_LIBRARY_H
