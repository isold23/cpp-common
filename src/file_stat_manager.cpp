#include "file_stat_manager.h"

uint8_t CFileStatManager::Check(const char* lszFileName)
{
    if(lszFileName != NULL) {
        strncpy(mszFileName, lszFileName, FILE_NAME_LEN);
    }
    
    if(stat_fn(mszFileName, &moStatus) == 0) {
        mbooChecked = SUCC;
        return SUCC;
    } else {
        mbooChecked = FAIL;
        perror("CFileStatManager::Check stat_fn.");
        return FAIL;
    }
}

uint16_t CFileStatManager::GetFileAccessPermission()
{
    if(BeChecked() == SUCC) {
        return moStatus.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID);
    } else {
        return 0;
    }
}

uint8_t CFileStatManager::SetFileAccessPermission(uint16_t lu16FileAccessPermission)
{
    if(BeChecked() == SUCC) {
        if(chmod_fn(mszFileName, lu16FileAccessPermission) == 0) {
            return SUCC;
        } else {
            perror("CFileStatManager::SetFileAccessPermission chmod_fn.");
            return FAIL;
        }
    } else {
        return FAIL;
    }
}

uint8_t CFileStatManager::ChangeFileOwner(uid_t uid)
{
    if(BeChecked() == SUCC) {
        if(chown_fn(mszFileName, uid, -1) == 0) {
            return SUCC;
        } else {
            perror("CFileStatManager::ChangeFileOwner chown_fn.");
            return FAIL;
        }
    } else {
        return FAIL;
    }
}
