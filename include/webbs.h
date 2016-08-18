#define WWW_CODE

#include "config.h"
#include "bbs/bbs.h"
#include "bbs/vote.h"
#include "struct.h"
#include "prototypes.h"
#include "macro.h"
#include "sysdep.h"
#include "consts.h"
#include <time.h>
#include <utime.h>
#include <sys/types.h>

/* from webbs.c */
/* secname[i][0] 存放讨论区分类名，如“电脑科技” */
/* secname[i][1] 存放相应分类的描述，如“[院系][校园]” */
/* secode[i] 相应分类的符号标记，如0abc，0对应i为0的“BBS系统“，abc对应该 */
/* 分类中讨论区可拥有的三种属性，如a为“[本站]”。 */
extern int SECNUM;
extern char seccode[36][5];
extern char secname[36][2][20];
extern char host[][16];		/* 校内host表 */
extern int loginok;
extern struct userec currentuser; /* 当前用户信息 */
extern struct user_info *u_info; 
extern struct UTMPFILE *shm_utmp; /* 共享内存信息 */
extern struct BCACHE *shm_bcache;
extern struct UCACHE *shm_ucache;
extern char datestring[30];	/* 时间buf */
extern char *vote_type[5];	/* 投票类型 */

/* from bcache.c */
/* local variables of bcache.c */
/* extern char brc_buf[BRC_MAXSIZE]; */
/* extern int brc_cur, brc_size, brc_changed; */
/* extern char brc_name[BFNAMELEN]; */
/* extern int brc_list[BRC_MAXNUM], brc_num; */
/* 当前讨论区，多处使用 */
extern char currboard[STRLEN];

/* from userinfo.c */
/* local variables of userinfo.c  */
/* extern struct override fff[200]; */
/* extern int friendnum; */
/* extern struct override bbb[MAXREJECTS]; */
/* extern int badnum; */
/* 封禁用户信息，maintain和userinfo中交互 */
extern struct denyheader denyuser[256]; 
extern int denynum;

/* from http.c */
/* http请求产生的参数名字与键值 */
extern char parm_name[256][80], *parm_val[256];
extern int parm_num;

/* from script.c */
/* hs_assign通用缓冲区，类似genbuf */
extern char hs_genbuf[8][128];

/* from bbs.h */
extern int flock(int fd, int operation);


#include <stdarg.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <sys/errno.h>

#define SERVER_PROTOCOL "HTTP/1.0"
#define HTTPD_VERSION "Sanry-httpd/0.1"
#define SET_EFFECTIVE_ID

/* 通用缓冲区 */
char genbuf[256];

/* 信号使用 */
int restart_pending, shutdown_pending;
/* http请求相关 */
char host_header[256];			/* 用户使用的域名 */
char fromhost[256];
char *stuff_ent, *query_string, *request_method;
char *http_cookie, content_type[256];
char if_modified_since[256], last_modified[256];
int content_length;
/* For Content-Range */
off_t range_start, range_end;

/* server端口 */
unsigned int server_port;
/* 已成为另一个genbuf */
char ULIST[STRLEN];


#define ACCESS_LOG	1
#define HTTPD_LOG	2
#define ERROR_LOG	3

#define ELOG_WARNING	1
#define ELOG_ERROR	2
#define ELOG_FATAL	3
