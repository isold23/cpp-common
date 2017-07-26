/******************************************************************
author: isold.wang@gmail.com
******************************************************************/

#include "dynamic_lib.h"

CDynamicLib::CDynamicLib(void)
{
    mhDynamic = NULL;
}

CDynamicLib::~CDynamicLib(void)
{
    CloseDynamicLib();
}

bool CDynamicLib::LoadDynamicLib(const char* apFileName)
{
    char lszName[512];
    memset(lszName, 0, 512);
#ifdef WIN32
    sprintf(lszName, "%s%s", apFileName, ".dll");
    mhDynamic = LoadLibrary(lszName);
#else
    sprintf(lszName, "%s%s", apFileName, ".so");
    mhDynamic = dlopen(lszName, RTLD_NOW);
    
    if(NULL == mhDynamic) {
        printf("CDynamicLib::LoadDynamicLib error:%s!", dlerror());
        return false;
    }
    
#endif
    return true;
}

void* CDynamicLib::GetDynamicLibFun(const char* apFunName)
{
    if(NULL == mhDynamic) {
        return NULL;
    }
    
#ifdef WIN32
    return GetProcAddress(mhDynamic, apFunName);
#else
    return dlsym(mhDynamic, apFunName);
#endif
}

void CDynamicLib::CloseDynamicLib()
{
    if(NULL != mhDynamic) {
#ifdef WIN32
        FreeLibrary(mhDynamic);
#else
        dlclose(mhDynamic);
#endif
        mhDynamic = NULL;
    }
}
