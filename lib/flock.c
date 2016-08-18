/*
   a flock implemention
   Firebird BBS for Windows NT
   Copyright (C) 2000, COMMAN,Kang Xiao-ning, kxn@student.cs.tsinghua.edu.cn

*/

#ifdef CYGWIN

#include <fcntl.h>
//#include <windows.h>
#include "sysdep.h"

/*
int
flock(int fd, int operation)
{
        int flag = 0;
        const DWORD len = 0xffffffff;
        OVERLAPPED offset;

        memset(&offset, 0, sizeof(offset));
        if (operation & LOCK_UN) {      
                return (UnlockFileEx((HANDLE)fd, 0, len, len, &offset)) ? 
			0 : -1;
        } else {                        
                if (operation & LOCK_NB)
                        flag |= LOCKFILE_FAIL_IMMEDIATELY;

                if (operation & LOCK_EX)
                        flag |= LOCKFILE_EXCLUSIVE_LOCK;

                return (LockFileEx((HANDLE)fd, flag, 0, len, len, 
			&offset)) ? 0 : -1;
        }
}
*/


#include <errno.h>
int flock(int fd,int operation)
{
	struct flock lock;

	memset(&lock, 0, sizeof(lock));
	if (operation & LOCK_EX) lock.l_type = F_WRLCK;
	if (operation & LOCK_UN) lock.l_type = F_UNLCK;
	if (operation & LOCK_NB)
		return fcntl(fd,F_SETLK,&lock);
	else
		return fcntl(fd,F_SETLKW,&lock);
}

#endif /* CYGWIN */
