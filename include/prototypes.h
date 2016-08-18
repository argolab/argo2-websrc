/* use for test*/
void test();
/* attach.c */
int getattachorig(char *board, char *fname, struct attacheader *ah);
int getattach(char *board, char *fname, struct attacheader *ah);
int delete_attach(void);
int edit_uploadext(void);
int modify_uploadext(void);
void show_attach(char *board, char *attfname);

/* announce.c */
int show_annpath(void);
int show_annfile(void);
char *anno_path_of(char *board);
char *getbfroma(char *path);
int announce_search(void);
int m_announce_stat(void);

/* bbsdaemon.c */
void add_msg(int);
void abort_program(int);
int daemon_main(void);
/* bcache.c */
char *brc_getrecord(char *ptr, char *name, int *pnum, int *list);
char *brc_putrecord(char *ptr, char *name, int num, int *list);
void brc_update(void);
int brc_initial(char *boardname);
int brc_locate(int num);
int brc_insert(int num);
void brc_addlist(char *filename);
int brc_unread(char *filename);
int brc_unreadt(int ftime);
void brc_clear(void);
int get_lastpost(char *board, int *lastpost, int *total);
int update_lastpost(char *board);
struct boardheader *getbcache(char *board);
/* boards.c */
int isOutgoingBoard(char *board);
int junkboard(char *board);
int anonyboard(char *board);
int getbnum(char *board);
int getsecnum(char *board);
char *getsecname(char *board);
char *sec(int c);
struct fileheader *get_file_ent(char *board, char *file);
char *flag_str(int access);
char *flag_str2(int access, int has_read);
int cmpboard(const void *b1, const void *b2);
int filenum(char *board);
int board_read(char *board);
int show_section_frame(void);
//tmp..
int show_main_frame(void);
//
int show_all_section(void);
int show_section(void);
int show_section_title(void);
int show_board(void);
int show_article(void);
int show_brddigest(void);
int show_digest(void);
int show_board_topic(void);
int show_good_boards(void);
char *topic_stat(struct fileheader *data, int from, int total);
int cmpfilename(void *filename, void *fh);
int cmparticleid(void *id, void *fh);
int show_topic(void);
void show_file_stuff(char *board, struct fileheader *x);
int show_all_boards(void);
int show_activeboard(void);
int select_board(void);
int show_board_note(void);
void brdfind_form(char *board);
int find_inboard(void);
int clear_read_flag(void);
int site_search(void);
int search_site(char *id, char *pat, char *pat2, char *pat3, int dt);
int note_flag(char *bname, char val);
int bin_get_record(char *filename, char *file, struct fileheader *x, int size, int total);
/* comm_list.c */
int do_stuff(void);
void cmd_sort();
int cmd_search(char *name);
/* config.c */
char *get_conf(const char *item);
int conf_error(char *msg);
void check_conf(void);
/* core.c */
int readsock(int fd);
/* file.c */
int file_has_word(char *file, char *word);
int f_append(char *file, char *buf);
struct stat *f_stat(char *file);
int post_inform(char *board, char *title, char *filename);
int post_article(char *board, char *title, int fd, char *id, char *nickname, char *ip, int sig);
int sig_append(FILE *fp, char *id, int sig);
int file_copy(FILE *fp1, FILE *fp2);
char *eff_size(char *file);
int getfilename(char *basedir, char *filename, int flag, int *id);
int getnewfilename(char *board);
/* html.c */
int set_color(int clr);
int get_color();
char *get_content_type(char *extname);
char *nohtml(char *s);
char *noansi(char *s);
int strsncpy(char *s1, char *s2, int n);
char *strright(char *s, int len);
char *ltrim(char *s);
char *rtrim(char *s);
void http_term(void);
void http_quit(void);
void http_fatal(char *fmt, ...);
char *strnncpy(char *s, int *l, char *s2);
int hsprintf(char *s, char *fmt, ...);
int hprintf(char *fmt, ...);
int hhprintf(char *fmt, ...);
void http_init(void);
int __to16(int c);
int __unhcode(char *s);
char *void1(unsigned char *s);
char *userid_str(char *s);
char *nbsp(const char *s);
char *getdatestring(time_t now);
char *inttostr(int i);
void form_header(char *href[], int *width);
void form_content(char *href[], int *width, int highlight);
void form_foot(int start, int total, int lines, char *fmt, ...);
void postheader(void);
void form_title(struct title_t *x);
void rightclick(void);
void form_view_header(void);
void form_read_header(char *msg);
void mail_read_header(int mailnum);
void article_read_header(char *board, struct fileheader *x);
void form_view_foot(void);
void form_read_foot(void);
void form_post_header(char *board, char *title, int articleid);
void form_post_foot(char *board, char *title);
void mail_post_header(struct fileheader *x);
void mail_post_foot(void);
char *UTF8(char *url);
void section_title(void);
void section_header(void);
void section_content(int i);
/* http.c */
char *getsenv(char *s);
void parm_add(char *name, char *val);
char *getparm(char *var);
int set_my_cookie(void);
void http_parm_free();
/* list.c */
int is_rejected(char *id);
int show_friends(void);
int show_online_user(void);
int show_all_user(void);
int show_bar(void);
int count_www(void);
int top_ten(void);
int board_top_ten(void);
/* load.c */
int chkload(int limit);
/* log.c */
int fatal(char *msg);
void prepare_log(void);
void do_log2(int log_type, int log_param, char *fmt, ...);
/* login.c */
int record_badlogin(char *username);
int bbs_login(char *id, char *pw);
int wwwlogin(struct userec *user);
void check_multi(char *id);
int is_bansite(char *ip);
int do_login(void);
int do_logout(void);
int show_index(void);
pid_t get_daemon();
int validate_ip_range(char *);
/* mail.c */
int count_mails(char *id, int *total, int *unread);
int count_new_mails(void);
int mail_file(char *tmpfile, char *userid, char *title);
int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig, int re);
int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);
void setmdir(char *buf, char *userid);
int getmailboxsize(unsigned int userlevel);
int getmailsize(char *userid);
int show_all_mails(void);
int show_mail(void);
int show_newmail(void);
int write_mail(void);
int do_send_mail(void);
int del_mail(void);
/* maintain.c */
int do_deny(void);
void deny_form(char *board);
void deny_modify_inform(char *board, char *user, char *exp, int dt);
void deny_add_inform(char *board, char *user, char *exp, int dt);
int deny_list(void);
int deny_attach(void);
int do_undeny(void);
void undeny_inform(char *board, char *user);
int m_deny(void);
void m_deny_add_form(char *board, char *userid, int num);
void m_deny_add_inform(char *board, char *user, char *exp, int dt);
int m_undeny(void);
void m_undeny_inform(char *board, char *user);
int m_show_board(void);
int board_manage(void);
int do_del(char *board, char *file);
int do_set(char *board, char *file, int flag);
int change_note(void);
int save_note(char *path);
char *getdatestr(time_t now);
int edit_pattern(void);
int modify_pattern(void);
int restart_httpd(void);

/* menu.c */
int show_menu(void);
int show_notepad(void);
int show_foot(void);
/* message.c */
int do_send_msg(char *myuserid, int mypid, char *touserid, int topid, char *msg);
int send_msg(void);
int get_msg(void);
int show_msg(void);
int del_msg(void);
int mail_msg(void);
/* modetype.c */
char *ModeType(int mode);
/* pass.c */
char *crypt1(char *buf, char *salt);
void igenpass(const char *passwd, const char *userid, unsigned char md5passwd[]);
int setpasswd(const char *passwd, struct userec *user);
void genpasswd(const char *passwd, unsigned char md5passwd[]);
int checkpasswd(const char *passwd, const char *test);
int checkpasswd2(const char *passwd, const struct userec *user);
int checkpasswd3(const char *passwd, const char *test);
/* perm.c */
int has_enter_perm(struct userec *user, char *board);
int has_post_perm(struct userec *user, char *board);
int has_mail_perm(struct userec *user);
int has_BM_perm(struct userec *user, char *board);
int has_read_perm(struct userec *user, char *board);
int has_post_perm1(struct userec *user, char *board);
int user_perm(struct userec *x, int level);
int deny_names(void *userid, void *dh);
int denynames(void *userid, void *dh);
int deny_me(char *bname);
/* post.c */
int write_article(void);
int submit_article(void);
int write_posts(char *board, unsigned int id);
int if_exist_id(unsigned int id);
int edit_article(void);
int update_form(char *board, char *file, char *title);
int copy_article(void);
int do_ccc(struct fileheader *x, char *board, char *board2);
int mail_article(void);
int do_fwd(struct fileheader *x, char *board, char *target);
int del_article(void);
/* process.c */
char *gmt_time(time_t time);
int http_msg(int msgno);
int header_request_handler(void);
int show_file(char *filename);
void doc_handler(void);
int header_param_handler(char*);
int process(void);
void bbs_daemon(void);
char *msg_str(int errcode);

/* record.c */
int get_record(char *filename, void *rptr, int size, int id);
int safe_substitute_record(char *direct, struct fileheader *fhdr, int ent, int sorted);

//void tmpfilename(char *filename, char *tmpfile, char *deleted);
int delete_record(char *filename, int size, int id);
int substitute_record(char *file, void *buf, int size, int num);
int get_records(char *filename, void *rptr, int size, int id, int number);
int get_num_records(char *filename, int size);
//int search_record(char *filename, void *rptr, int size, int (*fptr)(void *, void *), void *farg);
int safewrite(int fd, void *buf, int size);
int append_record(char *filename, void *record, int size);
int apply_record(char *filename, int (*fptr)(void *, int), int size);
/* register.c */
int cmpregrec(void *username, void *rec);
int has_fill_form(void);
int count_same_reg(char *username, int type, int myecho);
int check_register_ok(void);
int show_register_form(void);
int id_with_num(char *userid);
int bad_user_id(char *userid);
int badymd(int y, int m, int d);
int m_userid_validcate(void);
int do_register(void);
int badstr(unsigned char *s);
int bad_email(unsigned char *s);
int newcomer(struct userec *x, char *words);
int adduser(struct userec *x);
int fill_register_form(void);
int check_if_ok(void);
int check_submit_form(void);
int m_activation();
int m_validate();
int get_code(struct userec *trec);
int countmails(void *uentp_ptr, int unused);
int multi_mail_check(char *email);
int countnetids(void *uentp_ptr, int unused);
int multi_netid_check(char *netid);

void send_regmail(struct userec *trec);
/* shm.c */
void resolve_utmp(void);
void update_utmp(void);
void resolve_ucache(void);
void resolve_bcache(void);
char *get_new_shm(int key, int size);
char *get_old_shm(int key, int size);
int get_shmkey(char *s);
void shm_init(void);
int init_no_http(void);
void *get_shm(int shmkey, int shmsize);
/* signal.c */
void sig_alarm(int sig);
void sig_restart(int sig);
void sig_term(int sig);
void die_child(int sig);
void my_signal(int signo, void *func);
void set_signals(void);
/* ucache.c */
int getusernum(char *id);
struct userec *getuser(char *id);
int search_ulist(struct user_info **uentp, char *id);
int user_init(struct userec *x, struct user_info **y);
int findnextutmp(char *id, int from);
int count_id_num(char *id);
int count_online(void);
int count_online2(void);
void setuserid(int num, char *userid);
/* unicode.c */
int urldecode(char *dst, const char *src, int bufflen);
/* userinfo.c */
int loaddenyuser(char *board);
int savedenyuser(char *board);
int loadfriend(char *id);
int isfriend(char *id);
int loadbad(char *id);
int isbad(char *id);
int is_maildeny(char *id);
int save_user_data(struct userec *x);
int count_life_value(struct userec *urec);
int countexp(struct userec *x);
int countperf(struct userec *x);
int modify_mode(struct user_info *x, int newmode);
char *cperf(int perf);
char *c_exp(int exp);
char *horoscope(int month, int day);
int query_user(void);
int show_special(char *id2);
int change_nick(void);
int change_signature(void);
void save_signature(char *path);
int cmpuser(const void *a, const void *b);
int change_passwd(void);
int add_bad(void);
int add_friend(void);
int list_bad(void);
int list_friend(void);
int del_friend(void);
int del_bad(void);
int change_info(void);
int check_info(void);
int change_plan(void);
int save_plan(char *plan);
int change_cloak(void);
int change_parm(void);
int save_parm(void);
int config_interface(void);
int save_if_set(char *path, int t_lines, int link_mode, int def_mode);
int list_favorbrd(void);
int favorbrd_submit(void);
int ismybrd(char *board);
int add_favorbrd(void);
int ranklist(void);
int search_online(void);
int count_signature();
int auth_fillform(struct userec *u, int unum);
int check_auth_info(struct new_reg_rec *regrec);
int countauths(void *uentp_ptr, int unused);
int multi_auth_check(char auth[MD5_PASSLEN]);
int valid_day(int year, int month, int day);
/* vote.c */
int show_vote_list(void);
int cmpvuid(void *userid, void *uv);
void multivote(struct ballot *uv);
void valuevote(struct ballot *uv);
void askvote(struct ballot *uv);
int do_vote(void);
int post_vote(void);
int board_has_vote(char *);
/* webbs.c */

char *Ctime(time_t t);
char *Cdtime(time_t t);

/* report.c */
void do_securityreport(char *str, struct userec *userinfo, int fullinfo, char *addinfo);
void securityreport(char *str);
void securityreport2(char *str, int fullinfo, char *addinfo);
void getuinfo(FILE *fn, struct userec *userinfo);
int post_security_inform(char *title, char *filename);
void system_init();
void init_all();
int getGroupSet();
int create_board();
int cmpboardname(const void*, const void*);

void reapchild(int);
void modify_user_mode(int);
int upload_manage(void);
void maintaining(void);

/* script.c */
void hs_init(int unit);
char *safe_strcat(char *str1, const char *str2, int catlen, int *len);
void hs_assign(char *pat, char *word);
int hs_setfile(char *filename);
void hs_setloop(char *tag);
void hs_doloop(char *tag);
void hs_end(void);

/* filter.c */
void init_filter();
int regex_strstr(const char *haystack);
int str_gets(char *text, char *line, int size);
int check_text(char *title, char *content);

/* boardsuggester.c */
int m_board_suggester();

/* ads_management.c */
int ads_management();
int ads_update();
int ads_click();
