#include "webbs.h"

                        
int 
getusernum (id)
char *id; {
        int i;
        if (id[0] == '\0') return -1;

	resolve_ucache();
        for (i = 0; i < MAXUSERS; i ++) {
                if(!strcasecmp(shm_ucache->userid[i], id))
			return i;
        }
        return -1; 
}

struct userec *
getuser (id)
char *id; {
        static struct userec urec;
        int uid;

	resolve_ucache();

        if ((uid = getusernum(id)) < 0) return NULL;

	return (get_record(PASSFILE, &urec, sizeof (urec), uid + 1) == -1 || urec.userid[0] == '\0') ? NULL : &urec;
}

/* Add by Henry, 2002.12.11 */
int 
search_ulist (uentp, id)
struct user_info **uentp;
char *id; {
	int i;

	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = &(shm_utmp->uinfo[i]);
		if (!strcasecmp((*uentp)->userid, id))
			return i;
	}
	return -1;
}

int 
user_init (x, y)
struct userec *x;
struct user_info **y; {
        struct userec *x2;
        char id[20], num[20];
        int i, key;   
        strsncpy(id, getparm("utmpuserid"), 13);
        strsncpy(num, getparm("utmpnum"), 12);
        if(id[0]==0 || num[0]==0) return 0;
        key=atoi(getparm("utmpkey"));
        i=atoi(num);
        if(i<0 || i>=MAXACTIVE) return 0;
        (*y)=&(shm_utmp->uinfo[i]);
        if(strncmp((*y)->from, fromhost, 24)) return 0;
//      if((*y)->utmpkey != key) return 0;
        if((*y)->active==0) return 0;
        if((*y)->userid[0]==0) return 0;
        if (!((*y)->mode & WWW)) return 0;
        if(!strcasecmp((*y)->userid, "new") || !strcasecmp((*y)->userid, "guest")) return 0;
        x2=getuser((*y)->userid);
        if(x2==0) return 0;
        if(strcmp(x2->userid, id)) return 0;
        memcpy(x, x2, sizeof(*x));
        return 1;
}

int 
findnextutmp (id, from)
char *id;
int from; {
        int i;
        if(from<0) from=0;
        for(i=from; i<MAXACTIVE; i++)
                if(shm_utmp->uinfo[i].active)
                        if(!strcasecmp(shm_utmp->uinfo[i].userid, id)) return i;
        return -1;
}

int 
count_id_num (id)
char *id; {
        int i, total=0;
        for(i=0; i<MAXACTIVE; i++)
                if(shm_utmp->uinfo[i].active && !strcasecmp(shm_utmp->uinfo[i].userid, id)) total++;
        return total;
}  

int count_alluser() {
	int i, total = 0;

	for (i = 0; i < MAXUSERS; i++) 
		if (shm_ucache->userid[i][0] != 0) total++;
	return total;
}

int 
count_online () {
        int i, total=0;
        for(i=0; i<MAXACTIVE; i++)
                if(shm_utmp->uinfo[i].active) total++;
        return total;
}

int 
count_online2 () {
        int i, total=0;
        for(i=0; i<MAXACTIVE; i++)
                if(shm_utmp->uinfo[i].active && shm_utmp->uinfo[i].invisible==0) total++;
        return total;
}

void 
setuserid (num, userid)
int num;
char *userid;
{
        if (num >= 0 && num < MAXUSERS) {
                if (num >= shm_ucache->number)
                        shm_ucache->number = num + 1;
                strlcpy(shm_ucache->userid[num], userid, IDLEN + 1);
        }
}

/*
int      
fillucache(void *p, int uid)
{               
        struct userec *uentp = p;
                
        if (usernumber < MAXUSERS) {
                strlcpy(uidshm->userid[usernumber], uentp->userid, IDLEN + 1);
                uidshm->userid[usernumber++][IDLEN] = '\0';
        }
        return 0;
}
*/
