#include "webbs.h"

struct UTMPFILE *shm_utmp;

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

char *
get_old_shm (key, size)
int key;
int size; {
        int id;
        id=shmget(key, size, 0);
        if(id<0) return 0;
        return shmat(id, NULL, 0);
}

int main(int argc, char *argv[]) {
	int i;

	shm_utmp = (struct UTMPFILE *) get_old_shm(UTMP_SHMKEY, sizeof(struct UTMPFILE));
	for (i = 0; i < MAXACTIVE; i++) {
		if (shm_utmp->uinfo[i].active &&
			!strcmp(shm_utmp->uinfo[i].userid, argv[1]))
		show_status(&(shm_utmp->uinfo[i]));
	}

}

int show_status(struct user_info *uentp) {
	printf("%ud\n", uentp->mode);
}
