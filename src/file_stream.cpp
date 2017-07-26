#include <execinfo.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <std::string.h>
#include "file_stream.h"

//得到当前应用程序的路径
void CFileStream::GetAppPath(char* apPath, int aiLen)
{
    memset(apPath, 0, aiLen);
#ifdef WIN32
    GetModuleFileName(NULL, (LPSTR)apPath, aiLen);
#else
    char lcAppPath[256 + 1] = "";
    sprintf(lcAppPath, "/proc/self/exe");
    readlink(lcAppPath, apPath, aiLen - 1);
#endif
}

//创建路径
void CFileStream::CreatePath(char* apPath)
{
#ifdef WIN32
    CreateDirectory(apPath, NULL);
#else
    //mkdir函数的第二个参数用于设置创建文件夹的权限，
    //权限采用8进制，设置为777表示所有用户都可以查看
    mkdir(apPath, 511);
#endif
}

