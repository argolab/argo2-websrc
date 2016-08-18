#ifdef BBSMAIN

/* announce.c */
extern struct anninfo ainfo;

/* bbs.c */
extern char *currBM;
extern char currboard[BFNAMELEN + 1];
extern int current_bm;
extern struct userec currentuser;
extern int digestmode;
extern int local_article;
extern char genbuf[BUFLEN];
extern int in_mail;
extern struct postheader header;
extern char quote_title[120], quote_board[120];
extern char quote_file[120], quote_user[120];
extern char save_title[TITLELEN];
extern int selboard;

/* bbsd.c */
extern int bbsport;

/* bcache.c */
extern struct BCACHE *brdshm;
extern struct boardheader *bcache;
extern int numboards;

/* boards.c */
extern char *restrict_boards;

/* edit.c */
extern int editansi;

/* io.c */
extern int KEY_ESC_arg;

/* main.c */
extern char BoardName[STRLEN];
extern jmp_buf byebye;
extern int count_friends;
extern int count_users;
extern int guestuser;
extern char fromhost[60];
extern int iscolor;
extern time_t login_start_time;
extern int numofsig;
extern int showansi;
extern sigjmp_buf jmpbuf;
extern struct user_info uinfo;
extern char ULIST[STRLEN];
extern int utmpent;

/* maintain.c */
extern int usernum;

/* read.c */
extern char currdirect[PATH_MAX + 1];

/* report.c */
#ifdef MSGQUEUE
extern int msqid;
#endif

/* shm.c */
extern struct BCACHE *brdshm;
extern struct UCACHE *uidshm;
extern struct UTMPFILE *utmpshm;
extern struct FILESHM *welcomeshm;
extern struct FILESHM *goodbyeshm;
extern struct FILESHM *issueshm;
extern struct STATSHM *statshm;
extern struct ACSHM *movieshm;
extern struct ELSHM *endline_shm;

/* stuff.c */
extern char datestring[30];

/* term.c */
extern int t_lines;
extern int t_columns;

/* ucache.c */
extern int usernumber;

#endif
