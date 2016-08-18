/*
    Firebird BBS for Windows NT
    Copyright (C) 2000, COMMAN,Kang Xiao-ning, kxn@student.cs.tsinghua.edu.cn

  A dirty implementation of share memory on Win32 by COMMAN <kxn@263.net>
*/

#ifdef CYGWIN

#include <windows.h>
#include <string.h>
#include "sysdep.h"

int
shmget(int key, int size, int flag)
{
        HANDLE hMap;
        char fname[9];

        sprintf(fname, "SHM%05X", key);
        if (flag & IPC_CREAT) {
                hMap = CreateFileMapping((HANDLE)0xFFFFFFFF,
                                         NULL,
                                         PAGE_READWRITE,
                                         0,
                                         size,
                                         fname);
        } else {
                hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, fname);
        }

        return (hMap == 0) ? -1 : (int) hMap;
}

void *
shmat(int shmid, void *addr, int flag)
{
        return MapViewOfFile((HANDLE)shmid, FILE_MAP_ALL_ACCESS, 0, 0, 0);
}

int
shmdt(void *addr)
{
        return (UnmapViewOfFile(addr) == 0) ? -1 : 0;
}

#endif /* CYGWIN */
