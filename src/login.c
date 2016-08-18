#include "webbs.h"

#define BAD_HOST                BBSHOME"/etc/bad_host"


int 
record_badlogin (baduserid)
char *baduserid;
{
	FILE *fp;
	char buf[256], fname[256];
	time_t now;
	
	now = time(NULL);
	sprintf(buf, "%-12.12s  %24.24s %s [WWW]\n", baduserid,
		ctime(&now), fromhost);

	sethomefile(fname, baduserid, "logins.bad");
	if ((fp = fopen(fname, "a+")) != NULL) {
		fprintf(fp, "%s", buf);
		fclose(fp);
	}
	
	sprintf(fname, "%s/wwwlog/logins.bad", BBSHOME);
	if ((fp = fopen(fname, "a+")) != NULL) {
		fprintf(fp, "%s", buf);
		fclose(fp);
	}

	return 0;
}

int 
bbs_login (id, pw)
char *id;
char *pw;
{
	int n, t;
	char buf[256];
	struct userec *x;

	if (seek_in_file(BAD_HOST, fromhost)) {
		http_fatal
			("对不起, 本站不欢迎来自 [%s] 的登录. <br>若有疑问, 请与SYSOP联系.",
			 fromhost);
	}
	
	if (loginok && strcasecmp(id, currentuser.userid)) {
		http_fatal
		    ("系统检测到目前你的计算机上已经登录有一个帐号 %s，请先退出.(%s)",
		     currentuser.userid,
		     "选择正常logout, 或者关闭所有浏览器窗口");
	}
	x = getuser(id);
	if (x == NULL)
		http_fatal("错误的使用者帐号");
	if (strcasecmp(id, "guest")) {
//		if (validate_ip_range(fromhost) == 0) {
//			http_fatal("未加验证的ip地址[%s]，请使用guest登陆", fromhost);
//		}
		t = x->lastlogin;
		//babydragon: 首次登陆无此限制
		if (abs(t - time(0)) < 15 && x->numlogins > 1)
			http_fatal("两次登录间隔过密! 请15秒后再进行登陆");
		if (!checkpasswd2(pw, x)) {
			if (pw[0] != 0)
				sleep(2);
			sprintf(buf, "%s %s %s\n", Cdtime(time(0)), id,
				fromhost);
			file_append("wwwlog/badlogin.www", buf);
			record_badlogin(id);
			http_fatal
			    ("密码错误！如果这是你自己的id，请返回重新登录；如果这不是你的id，请不要再试别人的密码，系统会对每次登录都进行记录");
		}
		check_multi(x->userid);
		if (!user_perm(x, PERM_BASIC))
			http_fatal
			    ("此帐号已被停机, 若有疑问, 请用其他帐号在Complain版询问.");
		x->lastlogin = time(0);
		//save_user_data(x);
		x->numlogins++;
		strlcpy(x->lasthost, fromhost, sizeof(x->lasthost));
		save_user_data(x);
		currentuser = *x;
	}
	sprintf(buf, "%s %s %s\n", Ctime(time(0)), x->userid, fromhost);
	file_append("wwwlog/www.log", buf);
	sprintf(buf, "%s ENTER %-12s @%s [www]\n",
		getdatestring(time(0)), x->userid, fromhost);
	file_append("wwwlog/usies", buf);
	n = 0;
        if (!strcasecmp(id, "guest") || (!loginok && strcasecmp(id, "guest")))
 		wwwlogin(x);
	return 1;
}

int 
wwwlogin (user)
struct userec *user;
{
	FILE *fp;
	char buf[80];
	int pid, n, tmp;
	struct user_info *u;
	char file[256];

	if (!(currentuser.userlevel & PERM_LOGINOK)) {
		sethomefile(file, currentuser.userid, "register");
		if (file_exist(file)) {
			currentuser.userlevel |= PERM_DEFAULT;
			save_user_data(&currentuser);
		}
	}
	gethostname(genbuf, sizeof(genbuf));
	snprintf(ULIST, sizeof(ULIST), "UTMP.%s", genbuf);
	fp = fopen(ULIST, "a");
	flock(fileno(fp), LOCK_EX);

	for (n = 0; n < MAXACTIVE; n++) {
		u = &(shm_utmp->uinfo[n]);
		if ((!u->active || !u->pid) && u->deactive_time + 60 < time(NULL)) {
			u_info = u;
			memset(u, 0, sizeof (struct user_info));
			u->active = 1;
			u->uid = getusernum(user->userid) + 1;

			modify_user_mode(LOGIN);
			/* Henry: 咸鱼返生 */
			if (user_perm(&currentuser, PERM_SUICIDE)) {
				currentuser.userlevel &= ~PERM_SUICIDE;
				currentuser.userlevel |= PERM_MESSAGE;
				currentuser.userlevel |= PERM_SENDMAIL;
				save_user_data(&currentuser);
			}
			if (user_perm(&currentuser, PERM_LOGINCLOAK) &&
			    (currentuser.flags[0] & CLOAK_FLAG))
				u->invisible = YEA;
			u->pager = 0;
			if (currentuser.userdefine & DEF_FRIENDCALL)
				u->pager |= FRIEND_PAGER;
			if (currentuser.flags[0] & PAGER_FLAG) {
				u->pager |= ALL_PAGER;
				u->pager |= FRIEND_PAGER;
			}
			if (currentuser.userdefine & DEF_FRIENDMSG)
				u->pager |= FRIENDMSG_PAGER;
			if (currentuser.userdefine & DEF_ALLMSG) {
				u->pager |= ALLMSG_PAGER;
				u->pager |= FRIENDMSG_PAGER;
			}
			strsncpy(u->from, fromhost, 24);
			*(int *) (u->from + 32) = time(0);
			u->idle_time = time(0);
			strlcpy(u->username, user->username, NICKNAMELEN + 1);
			strlcpy(u->userid, user->userid, IDLEN + 2);
			tmp = rand() % 100000000;
//                      u->utmpkey=tmp;
			sprintf(buf, "%d", n);
			setcookie("utmpnum", buf);
			sprintf(buf, "%d", tmp);
			setcookie("utmpkey", buf);
			setcookie("utmpuserid", currentuser.userid);
			set_my_cookie();
			flock(fileno(fp), LOCK_UN);
			fclose(fp);
			u->hideip = 0;
			if (DEFINE(DEF_NOTHIDEIP)) u->hideip = 'N';
			if (DEFINE(DEF_FRIENDSHOWIP)) u->hideip = 'F';
			
			/* Henry: 移到最后，减少webbsd读取错误u_info的机会
				否则webbsd会获取到错误的userid */

			pid = get_daemon();
			if (pid == 0) http_fatal("无法登陆");
			u->pid = pid;
/* Henry: This is a experimental value to ensure daemon started */
			usleep(1000);
			update_utmp();

			return 0;
		}
	}

//	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	http_fatal("抱歉，目前在线用户数已达上限，无法登录。请稍后再来。");
	return -1;
}

void 
check_multi (id)
char *id;
{
	int i, total = 0;

	if (currentuser.userlevel & PERM_SYSOP)
		return;
	for (i = 0; i < MAXACTIVE; i++) {
		if (shm_utmp->uinfo[i].active == 0 || shm_utmp->uinfo[i].pid == 0)
			continue;
		if (!strcasecmp(shm_utmp->uinfo[i].userid, id))
			total++;
	}
	if (total >= 3)
		http_fatal
		    ("您已经登录了3个窗口。为了保证他人利益，此次连线将被取消。");
}

int
do_login()
{
        char id[IDLEN+2], pw[41];

	/* Henry: 发文时登陆需要置noinit位 */
        if (atoi(getparm("noinit")) == 0) init_all();

        strsncpy(id, getparm("id"), IDLEN+1);
        strsncpy(pw, getparm("pw"), 40);
	bbs_login(id, pw);
	if(strcmp("reg", getparm("type")) == 0) {
		redirect("bbsauth");
		return 0;
	}
	if (atoi(getparm("quick"))) {
		printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
		printf("<script>top.f4.location='bbsfoot'</script>\n");
		printf("<script>top.f2.location='bbsleft'</script>\n");
	} else
		redirect(FIRST_PAGE);
	printf("</html>\n");
	if (atoi(getparm("noinit")) == 0) http_quit();
        return 0;
}

pid_t get_daemon() {
/* Return value:
 *    pid if success, 0 if fail
 */
	pid_t pid;
	struct sockaddr_in sa;
	struct in_addr addr;
	int sockfd;

	pid = 0;
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) return 0;
	memset(&sa, 0, sizeof(struct sockaddr));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(server_port);
	inet_aton("127.0.0.1", &addr);
	memcpy(&sa.sin_addr, &addr, sizeof(addr));
	if (connect(sockfd, (struct sockaddr *) &sa, sizeof(sa)) < 0) 
		return 0;
	if (send(sockfd, "BBS\n", 4, 0) == 0)
		return 0;
	if (recv(sockfd, &pid, sizeof(pid_t), 0) == 0)
		return 0;
	close(sockfd);
	return pid;
}

int do_logout() {
	int pid;
	init_all();
	if(!loginok) http_fatal("你没有登录");
	pid=u_info->pid;
	if(pid>0) kill(pid, SIGHUP);
	update_utmp();
	setcookie("utmpkey", "");
	setcookie("utmpnum", "");
	setcookie("utmpuserid", "");
	setcookie("my_t_lines", "");
	setcookie("my_link_mode", "");
	setcookie("my_def_mode", "");
	if (atoi(getparm("quick"))) {
		printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
		printf("<script>top.f4.location='bbsfoot'</script>\n");
		printf("<script>top.f2.location='bbsleft'</script>\n");
		printf("<script>top.f3.location='bbssec'</script>\n");
	} else
		redirect("/main.html");
	http_quit();
	return 0;
}

int show_index() {
        int pid;
        init_all();
        if(!loginok) {
		redirect("/");
		return 0;
	}
        pid=u_info->pid;
        if(pid>0) kill(pid, SIGHUP);
        setcookie("utmpkey", "");
        setcookie("utmpnum", "");
        setcookie("utmpuserid", "");
        setcookie("my_t_lines", "");
        setcookie("my_link_mode", "");
        setcookie("my_def_mode", "");
        redirect("/");
        http_quit();
        return 0;
}

 /* cp from telnet src
  * modified by freestyler */
int 
validate_ip_range(char *name)
{
	char* 	fname	= "etc/auth_host";
	int 	nofile	= 1; /* the return value when auth_host file not found */
	FILE *list;
	char buf[40], *ptr;

	if ((list = fopen(fname, "r")) != NULL) {
		while (fgets(buf, 40, list)) {
			ptr = strtok(buf, " \n\t\r");
			if (ptr != NULL && *ptr != '#') {
				if (!strcmp(ptr, name)) {
					fclose(list);
					return 1;
				}
				if (ptr[0] == '-' &&
				    !strcmp(name, &ptr[1])) {
					fclose(list);
					return 0;
				}
				if (ptr[strlen(ptr) - 1] == '.' &&
				    !strncmp(ptr, name, strlen(ptr) - 1)) {
					fclose(list);
					return 1;
				}
				if (ptr[0] == '.' &&
				    strlen(ptr) < strlen(name)
				    && !strcmp(ptr,
					       name + strlen(name) -
					       strlen(ptr))) {
					fclose(list);
					return 1;
				}
			}
		}
		fclose(list);
		return 0;
	}
	return nofile;
}
