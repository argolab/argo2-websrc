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
/* secname[i][0] ������������������硰���ԿƼ��� */
/* secname[i][1] �����Ӧ������������硰[Ժϵ][У԰]�� */
/* secode[i] ��Ӧ����ķ��ű�ǣ���0abc��0��ӦiΪ0�ġ�BBSϵͳ����abc��Ӧ�� */
/* ��������������ӵ�е��������ԣ���aΪ��[��վ]���� */
extern int SECNUM;
extern char seccode[36][5];
extern char secname[36][2][20];
extern char host[][16];		/* У��host�� */
extern int loginok;
extern struct userec currentuser; /* ��ǰ�û���Ϣ */
extern struct user_info *u_info; 
extern struct UTMPFILE *shm_utmp; /* �����ڴ���Ϣ */
extern struct BCACHE *shm_bcache;
extern struct UCACHE *shm_ucache;
extern char datestring[30];	/* ʱ��buf */
extern char *vote_type[5];	/* ͶƱ���� */

/* from bcache.c */
/* local variables of bcache.c */
/* extern char brc_buf[BRC_MAXSIZE]; */
/* extern int brc_cur, brc_size, brc_changed; */
/* extern char brc_name[BFNAMELEN]; */
/* extern int brc_list[BRC_MAXNUM], brc_num; */
/* ��ǰ���������ദʹ�� */
extern char currboard[STRLEN];

/* from userinfo.c */
/* local variables of userinfo.c  */
/* extern struct override fff[200]; */
/* extern int friendnum; */
/* extern struct override bbb[MAXREJECTS]; */
/* extern int badnum; */
/* ����û���Ϣ��maintain��userinfo�н��� */
extern struct denyheader denyuser[256]; 
extern int denynum;

/* from http.c */
/* http��������Ĳ����������ֵ */
extern char parm_name[256][80], *parm_val[256];
extern int parm_num;

/* from script.c */
/* hs_assignͨ�û�����������genbuf */
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

/* ͨ�û����� */
char genbuf[256];

/* �ź�ʹ�� */
int restart_pending, shutdown_pending;
/* http������� */
char host_header[256];			/* �û�ʹ�õ����� */
char fromhost[256];
char *stuff_ent, *query_string, *request_method;
char *http_cookie, content_type[256];
char if_modified_since[256], last_modified[256];
int content_length;
/* For Content-Range */
off_t range_start, range_end;

/* server�˿� */
unsigned int server_port;
/* �ѳ�Ϊ��һ��genbuf */
char ULIST[STRLEN];


#define ACCESS_LOG	1
#define HTTPD_LOG	2
#define ERROR_LOG	3

#define ELOG_WARNING	1
#define ELOG_ERROR	2
#define ELOG_FATAL	3
