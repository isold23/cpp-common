#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_

#include <sys/stat.h>
#include "include.h"
#include "define.h"

#define stat_fn stat
#define lstat_fn lstat
#define fstat_fn fstat
#define chmod_fn chmod
#define fchmod_fn fchmod
#define lchmod_fn lchmod
#define fchmodat_fn fchmodat
#define chown_fn chown

class CFileStatManager
{
public:

    CFileStatManager()
    {
        Reset();
    }
    
    ~CFileStatManager()
    {
        Reset();
    }
    
    void Reset()
    {
        mbooChecked = FAIL;
        memset(&moStatus, 0, sizeof(stat_t));
        memset(mszFileName, 0, FILE_NAME_LEN);
    }
    
    uint8_t       Check(const char* pFileName = NULL);
    uint32_t      GetFileType();
    uint16_t      GetFileAccessPermission();
    uint8_t       SetFileAccessPermission(uint16_t);
    uint8_t       ChangeFileOwner(uid_t uid);
    
private:
    uint8_t       BeChecked()
    {
        return mbooChecked;
    }
    
private:
    typedef struct stat     stat_t;
    static const int        FILE_NAME_LEN = 255;
    char                    mszFileName[ FILE_NAME_LEN ];
    stat_t                  moStatus;
    uint8_t mbooChecked;
};

#endif // _FILEMANAGER_H_

