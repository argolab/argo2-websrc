#include "webbs.h"

void resolve_utmp(void) {
	if (shm_utmp == NULL) {
		shm_utmp = get_shm(UTMP_SHMKEY, sizeof(*shm_utmp));
	}
}

void update_utmp(void) {
	time_t now;
	struct user_info *uentp;
	int utmpfd, i;

	gethostname(genbuf, sizeof(genbuf));
	sprintf(ULIST, "UTMP.%s", genbuf);
	utmpfd = open(ULIST, O_RDWR|O_CREAT, 0600);
	if (utmpfd < 0)
		http_fatal("创建在线列表失败，请与系统维护联系");

	if (shm_utmp == NULL) {
		shm_utmp = get_shm(UTMP_SHMKEY, sizeof (*shm_utmp));
	}

	now = time(NULL);
	if (shm_utmp->uptime + 60 < now) {
		f_exlock(utmpfd);
		for (i = 0; i < USHM_SIZE; i++) {
			uentp = &(shm_utmp->uinfo[i]);
			if (uentp->active) {
				if (uentp->pid != 1 && kill(uentp->pid, 0) == -1) {
					memset(uentp, 0,
						sizeof(struct user_info));
					uentp->deactive_time = time(NULL);
					continue;
				/* 使用进程号1来表示无进程的状态,15分钟超时 */
				} else if (uentp->pid == 1 && now > uentp->idle_time + 15 * 60) {
					uentp->active = NA;
					uentp->deactive_time = now;
					uentp->pid = 0;
				}
			}
		}
		shm_utmp->usersum = count_alluser();
		shm_utmp->uptime = now;
		i = USHM_SIZE -1;
		while (i > 0 && shm_utmp->uinfo[i].active == 0)
			i --;
		ftruncate(utmpfd, 0);
		write(utmpfd, shm_utmp->uinfo, 
			(i + 1) * sizeof(struct user_info));
		f_unlock(utmpfd);
	}
	close(utmpfd);
}
		

void resolve_ucache(void)
{
        int fd, i, fd2;
	struct userec urec;
	time_t now;

        if (shm_ucache == NULL) {
                shm_ucache = get_shm(UCACHE_SHMKEY, sizeof (*shm_ucache));
        }
	now = time(NULL);
        if (!shm_ucache->updating && shm_ucache->uptime + 86400 < now) {
                if ((fd = filelock("ucache.lock", NA)) > 0) {
			shm_ucache->updating = 1;
			fd2 = open(PASSFILE, O_RDONLY);
			if (fd2 == -1) http_fatal("系统重要文件丢失");
			for (i = 0; i < MAXUSERS; i++) {
				if (read(fd2, &urec, sizeof(urec)) <= 0)
					break;
				strlcpy(shm_ucache->userid[i], urec.userid, 
					sizeof(shm_ucache->userid[i]));
			}
			close(fd2);
			shm_ucache->number = i;
			shm_ucache->uptime = now;
			close(fd);
			shm_ucache->updating = 0;
		}
	}
}

void resolve_bcache(void) {
	struct stat st;
	time_t now;
	int fd, i;
	FILE *fp;
	struct boardheader bh;

	if (shm_bcache == NULL) 
                shm_bcache = get_shm(BCACHE_SHMKEY, sizeof (*shm_bcache));

        now = time(NULL);
        if (stat(BOARDS, &st) < 0) {
                st.st_mtime = now - 3600;
        }

        if (shm_bcache->updating != YEA && (shm_bcache->uptime < st.st_mtime || shm_bcache->uptime < now - 3600)) {
                if ((fd = filelock("bcache.lock", NA)) > 0) {
			if ((fp = fopen(BOARDS, "r")) != NULL) {
				shm_bcache->updating = YEA;
				for (i = 0; i < MAXBOARD; i++) {
					if (fread(&bh, 1, sizeof(bh), fp) != sizeof(bh))
						break;
					memcpy(&(shm_bcache->bcache[i]), &bh, sizeof(bh));
					update_lastpost(bh.filename);
				}
				fclose(fp);
	                        shm_bcache->number = i;
        	                shm_bcache->uptime = now;
				shm_bcache->updating = NA;
			}
                        close(fd);
                }
        }
}


char *
get_new_shm (key, size)
int key;
int size; {
        int id;
        id=shmget(key, size, IPC_CREAT | IPC_EXCL | 0640);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

char *
get_old_shm (key, size)
int key;
int size; {
        int id;
        id=shmget(key, size, 0);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}



int 
get_shmkey (s)
char *s; {
        int n=0; 
        while(shmkeys[n].key!=0) {
                if(!strcasecmp(shmkeys[n].key, s)) return shmkeys[n].value;
                n++;
        }
        return 0;
}

void 
shm_init ()
{
	shm_utmp = NULL;
	shm_ucache = NULL;
	shm_bcache = NULL;

	resolve_utmp();
	resolve_bcache();
	resolve_ucache();

/*
	shm_utmp= (struct UTMPFILE *) get_old_shm(UTMP_SHMKEY, sizeof(struct UTMPFILE));
	shm_bcache= (struct BCACHE *) get_old_shm(BCACHE_SHMKEY, sizeof(struct BCACHE));
	shm_ucache= (struct UCACHE *) get_old_shm(UCACHE_SHMKEY, sizeof(struct UCACHE));
        if(shm_utmp==0) http_fatal("shm_utmp error");
        if(shm_bcache==0) http_fatal("shm_bcache error");
        if(shm_ucache==0) http_fatal("shm_ucache error");
*/
}


int 
init_no_http () {
        srand(time(0)+getpid());
        chdir(BBSHOME);
        shm_init();
	return 0;
}

void *
get_shm(int shmkey, int shmsize)
{
	void *shmptr;
	int shmid;

	shmid = shmget(shmkey, shmsize, 0);
	if (shmid < 0) {
		shmid = shmget(shmkey, shmsize, IPC_CREAT | 0644);
		if (shmid < 0)
			return NULL;
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
			return NULL;
		memset(shmptr, 0, shmsize);
	} else {
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
			return NULL;
	}
	return shmptr;
}

void release_shm() {
	if (shm_utmp) shmdt(shm_utmp);
	if (shm_ucache) shmdt(shm_ucache);
	if (shm_bcache) shmdt(shm_bcache);
	shm_utmp = NULL;
	shm_ucache = NULL;
	shm_bcache = NULL;
}
