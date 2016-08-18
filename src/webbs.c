#include "webbs.h"

int SECNUM;
char seccode[36][5];
char secname[36][2][20];
char host[][16]={
        "202.116.64",
        "192.168",
        "172.16",
	"211.66.128",
	"172.18",
	"172.31",
	"219.222",
	"202.116.96",
	""
};

int loginok=0;

struct userec currentuser;
struct user_info *u_info; 
struct UTMPFILE *shm_utmp;
struct BCACHE *shm_bcache;
struct UCACHE *shm_ucache;
char datestring[30];

char *vote_type[] = { "是非", "单选", "复选", "数字", "问答" };

void system_init() {
        if (seek_in_file("etc/bad_host", fromhost))
                http_fatal("从此IP的登陆被禁止");
        getGroupSet();
}

void init_all() {
        srand(time(0)*2+getpid());
        chdir(BBSHOME);
        http_init();
        seteuid(BBSUID);
        if(my_geteuid()!=BBSUID) http_fatal("uid error.");
        shm_init();
        system_init();
        loginok=user_init(&currentuser, &u_info);
        if (loginok == 0)
                u_info = NULL;
}
