#ifndef _FILE_LOCK_H_
#define _FILE_LOCK_H_

    inline int file_trylock(int fd)
    {
        struct flock  fl;
        memset(&fl, 0, sizeof(struct flock));
        fl.l_pid = getpid();
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_END;
        
        if(fcntl(fd, F_SETLKW, &fl) == -1) {
            TRACE(1, "lock failed. errno: "<<errno);
            return -1;
        }
        
        return 0;
    }
    
    inline int file_unlock(int fd)
    {
        struct flock  fl;
        memset(&fl, 0, sizeof(struct flock));
        fl.l_pid = getpid();
        fl.l_type = F_UNLCK;
        fl.l_whence = SEEK_END;
        
        if(fcntl(fd, F_SETLK, &fl) == -1) {
            TRACE(1, "file unlock failed. errno: "<<errno);
            return  -1;
        }
        
        return 0;
    }

#endif //_FILE_LOCK_H_
