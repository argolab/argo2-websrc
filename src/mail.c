#include "webbs.h"

int 
count_mails (id, total, unread)
char *id;
int *total;
int *unread; {
        struct fileheader x1;
        char buf[256];
        FILE *fp;
        *total=0;
        *unread=0;
        if(getuser(id)==0) return -1;
        sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper((int)id[0]), id);
        fp=fopen(buf, "r");
        if(fp==NULL) return -1;
        while(fread(&x1, sizeof(x1), 1, fp)>0) {
                (*total)++;
                if(!(x1.flag & FILE_READ)) (*unread)++;
        }
        fclose(fp);
	return 0;
}

int 
count_new_mails () {
        struct fileheader x1;
        int unread=0;
        char buf[1024];
        FILE *fp;
        if(currentuser.userid[0]==0) return 0;
	setmailfile(buf, ".DIR");
        fp = fopen(buf, "r");
        if (fp==0) return 0;
        while(fread(&x1, sizeof(x1), 1, fp)>0)
                if(!(x1.flag & FILE_READ)) unread++;
        fclose(fp);
        return unread; 
}


int
mail_file(char *tmpfile, char *userid, char *title)
{
        struct fileheader newmessage;
        int anonymousmail = 0;
        char maildir[STRLEN], filename[STRLEN];
	
        if (userid == NULL || userid[0] == '\0')
                return -1;
	
        memset(&newmessage, 0, sizeof (newmessage));
        strlcpy(newmessage.owner, anonymousmail ? currboard : currentuser.userid, sizeof(newmessage.owner));
        strlcpy(newmessage.title, title, sizeof(newmessage.title));

        create_maildir(userid);
        if (getfilename(maildir, filename, GFN_FILE | GFN_UPDATEID, &newmessage.id) == -1)
                return -1;
        strlcpy(newmessage.filename, strrchr(filename, '/') + 1, sizeof(newmessage.filename));
        if (f_cp(tmpfile, filename, O_CREAT) == -1) {
                return -1;
        }

        snprintf(genbuf, sizeof(genbuf), "%s/%s", maildir, DOT_DIR);
        newmessage.filetime = time(NULL);
        if (append_record(genbuf, &newmessage, sizeof(newmessage)) == -1)
                return -1;
	
        report("mailed %s ", userid);
        return 0;
}

int 
post_mail (userid, title, file, id, nickname, ip, sig, re)
char *userid;
char *title;
char *file;
char *id;
char *nickname;
char *ip;
int sig;
int re; {
        FILE *fp, *fp2;
        char buf3[BUFLEN], dir[STRLEN];
        struct fileheader header, x; 
	struct stat st;
        int t, i;
        if(strstr(userid, "@"))  
                return post_imail(userid, title, file, id, nickname, ip, sig);
	setmailpath(buf3, userid);
/* Rewrite by Henry 03.02.15 */
	if (stat(buf3, &st) < 0) {
		/* Henry: try to create the user mailbox */
		f_mkdir(buf3, 0755);
		if (stat(buf3, &st) < 0)	//This time I've no idea
			http_fatal("用户邮箱不存在，请与系统维护联系");
	}
			
	if (!S_ISDIR(st.st_mode))
		http_fatal("用户邮箱格式错误");
/* Rewrite end */
        memset(&header, 0, sizeof(header)); 
        strlcpy(header.owner, id, IDLEN + 1); 
	t = time(0);
        for(i=0; i<100; i++) {
                sprintf(buf3, "mail/%c/%s/M.%d.A", mytoupper(userid[0]), 
			userid, t);
                if(!file_exist(buf3)) break;
		t++;
        }
        if(i>=99) return -1;
        sprintf(header.filename, "M.%d.A", t);
	header.filetime = t;
        strsncpy(header.title, title, TITLELEN);
        fp=fopen(buf3, "w");
        if(fp==0) return -2;   
        fp2=fopen(file, "r");
        fprintf(fp, "寄信人: %s (%s)\n", id, nickname);
        fprintf(fp, "标  题: %s\n", title);
        fprintf(fp, "发信站: %s (%s)\n", BBSNAME, Ctime(time(0)));
        fprintf(fp, "来  源: %s\n\n", ip);
        if(fp2) {
                while(1) {
                        if(fgets(buf3, BUFLEN - 1, fp2)<=0) break;
	/* Henry: fprintf2会把颜色代码也算在78个字符内，导致提前换行 */
//                        fprintf2(fp, buf3);
			fprintf(fp, "%s", buf3);
                }
                fclose(fp2);
        }
        fprintf(fp, "\n--\n"); 
        sig_append(fp, id, sig);
        fprintf(fp, "\n\n\033[1;%dm※ 来源:．%s http://%s [FROM: %.20s]\033[m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
        fclose(fp);

	/* Henry: 标记已读 */
	if (re > 0) {
		setmailfile(dir, ".DIR");
		if (get_record(dir, &x, sizeof(x), re) != -1) {
			x.flag |= MAIL_REPLY;
			header.id = x.id;
			substitute_record(dir, &x, sizeof(x), re);
		}
	} else {
		header.id = t;
	}

	/* Henry: 给对方邮箱增添记录 */
        sprintf(dir, "mail/%c/%s/.DIR", mytoupper(userid[0]), userid);
	append_record(dir, &header, sizeof(header));

	// Added by betterman
	report("mailed '%s'", userid);
	
	return 0;
}

int 
post_imail (userid, title, file, id, nickname, ip, sig)
char *userid;
char *title;
char *file;
char *id;
char *nickname;
char *ip;
int sig; {
        FILE *fp1, *fp2;
        char buf[256];  
        if(strstr(userid, ";") || strstr(userid, "`"))
        {
// added by scs for log attack
                sprintf(buf, "%s %s %s attack from wwwmail:%s\n", Ctime(time(0)), id, fromhost, userid);
                file_append("wwwattack.log", buf);
                http_fatal("错误的收信人地址");
        }
        sprintf(buf, "sendmail -f %s.bbs@%s '%s'", id, BBSHOST, userid);
        fp2=popen(buf, "w");
        fp1=fopen(file, "r");
        if(fp1==0 || fp2==0) return -1;
        fprintf(fp2, "From: %s.bbs@%s\n", id, BBSHOST);
        fprintf(fp2, "To: %s\n", userid);
        fprintf(fp2, "Subject: %s\n\n", title);
        while(1) {
                if(fgets(buf, 255, fp1)==0) break;
                if(buf[0]=='.' && buf[1]=='\n') continue;
                fprintf(fp2, "%s", buf);
        }
        fprintf(fp2, "\n--\n");
        sig_append(fp2, id, sig);
        fprintf(fp2, "\n\n\033[1;%dm※ 来源:．%s http://%s [FROM: %.20s]\033[m\n", 31+rand()%7, BBSNAME, BBSHOST, ip);
        fprintf(fp2, ".\n"); 
        fclose(fp1);
        pclose(fp2);
	return 0;
}

void 
setmdir (buf, userid)
char *buf;
char *userid;
{
        sprintf(buf, "mail/%c/%s/.DIR", mytoupper(userid[0]), userid);
}

int 
getmailboxsize (userlevel)
unsigned int userlevel;
{
        if (userlevel & (PERM_SYSOP))
                return 5000;
        if (userlevel & (PERM_LARGEMAIL))
                return 3600;
        if (userlevel & (PERM_BOARDS | PERM_INTERNAL))
                return 2400;
        if (userlevel & (PERM_LOGINOK))
                return 1200;
        return 5;
}

int 
getmailsize (userid)
char *userid;
{
        struct fileheader fcache;
        struct stat st;
        char dirfile[STRLEN], mailfile[STRLEN];
        int fd, total = 0, i, count;  
        char *p;
        
        setmdir(dirfile, userid);
        fd = open(dirfile, O_RDWR);
        if (fd != -1) {
                char repbuf[1000];
   
                f_exlock(fd);
                fstat(fd, &st);
                count = st.st_size / sizeof (fcache);
                sprintf(repbuf, "count: %d", count);
                sprintf(mailfile, "mail/%c/%s/", mytoupper(userid[0]), userid);
                p = strrchr(mailfile, '/') + 1;
        
                for (i = 0; i < count; i++) {
                        if (lseek(fd, (off_t) (sizeof (fcache) * i), SEEK_SET) == -1)
                                break;
        
                        if (read(fd, &fcache, sizeof (fcache)) != sizeof (fcache))
                                break;

                        if (fcache.size <= 0) {
                                strcpy(p, fcache.filename);
                                if (stat(mailfile, &st) != -1) {
                                        fcache.size = (st.st_size > 0) ? st.st_size : 1;
                                } else {
                                        fcache.size = 1;
                                }
                        
                                if (lseek(fd, (off_t) (sizeof (fcache) * i), SEEK_SET) == -1)
                                        break;
                        
                                if (safewrite(fd, &fcache, sizeof (fcache)) != sizeof(fcache))
                                        break;
                        }
                        total += fcache.size;
                }
                f_unlock(fd);
                close(fd);
        }
                                 
        return total / 1024 + 1;
}

int show_all_mails() {
	FILE *fp;
	int i, start, total, my_t_lines;
	char dir[80];
	struct fileheader *data;
	init_all();
	if(!loginok) http_fatal("您尚未登录, 请先登录");
	modify_user_mode(RMAIL);
	start=atoi(getparm("start"));
	my_t_lines = atoi(getparm("my_t_lines"));
	if (my_t_lines < 10 || my_t_lines > 40) my_t_lines = 20;
	setmailfile(dir, ".DIR");
	if (!file_exist(dir)) http_fatal("您没有任何信件");
   	total=file_size(dir)/sizeof(struct fileheader);
	if(total<0 || total>30000) http_fatal("too many mails");
	if (total == 0) http_fatal("您没有任何信件");
   	data=(struct fileheader *)calloc(total, sizeof(struct fileheader));
   	if(data==0) http_fatal("memory overflow");
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("dir error");
	total=fread(data, sizeof(struct fileheader), total, fp);
	fclose(fp);
	if(start == 0 || start > total - 19) start = total - 19;
	if(start < 1) start = 1;
	hs_init(20);
	hs_setfile("bbsmail.ptn");
	hs_setloop("mainform");
      	for(i = start; i < start + 20 && i <= total; i++) {
		sprintf(hs_genbuf[0], "%d", i);
		hs_assign("INDEX", hs_genbuf[0]);
		if (data[i-1].flag & FILE_READ) {
			if (data[i-1].flag & FILE_MARKED & MAIL_REPLY)
				hs_assign("FLAG", "b");
			else if (data[i-1].flag & FILE_MARKED)
				hs_assign("FLAG", "m");
			else if (data[i-1].flag & MAIL_REPLY)
				hs_assign("FLAG", "r");
			else	hs_assign("FLAG", "&nbsp");
		} else {
			if (data[i-1].flag & FILE_MARKED)
				hs_assign("FLAG", "M");
			else	hs_assign("FLAG", "N");
		}
		sprintf(hs_genbuf[1], "%12.12s", Cdtime(data[i-1].filetime));
		hs_assign("DATE", hs_genbuf[1]);
		snprintf(hs_genbuf[2], sizeof(hs_genbuf[2]), "<a href=bbsmailcon?start=%d>%s%*s</a>",
			i,
			strncmp("Re: ", data[i-1].title, 4)?
			"★ ":"", TITLELEN, void1(data[i-1].title));
		hs_assign("TITLE", hs_genbuf[2]);
		sprintf(hs_genbuf[3], "<a href=bbsqry?userid=%s>%s</a>",
			data[i-1].owner, data[i-1].owner);
		hs_assign("SENDER", hs_genbuf[3]);
		hs_doloop("mainform");
      	}
      	free(data);

        sprintf(hs_genbuf[0], "%d", total);
        hs_assign("TOTAL", hs_genbuf[0]);
        sprintf(hs_genbuf[1], "%d", my_t_lines);
        hs_assign("PERPAGE", hs_genbuf[1]);
        sprintf(hs_genbuf[2], "%d", (start + my_t_lines - 2) / my_t_lines + 1);
        hs_assign("PAGE", hs_genbuf[2]);
        sprintf(hs_genbuf[3], "%d", (total - 1) / my_t_lines + 1);
        hs_assign("PGTOTAL", hs_genbuf[3]);
        if (start == 1)
                strcpy(hs_genbuf[4], "<font color=ff0000>首页 上一页</font>");
        else
                sprintf(hs_genbuf[4], "<a href=bbsmail?start=1>首页</a> <a href=bbsmail?start=%d>上一页</a>",
                        start - my_t_lines);
        hs_assign("PGBUTTONUP", hs_genbuf[4]);
        if (start >= total - my_t_lines + 1)
                strcpy(hs_genbuf[5], "<font color=ff0000>下一页 末页</font>");
        else
                sprintf(hs_genbuf[5], "<a href=bbsmail?start=%d>下一页</a> <a href=bbsmail?start=%d>末页</a>",
                        start + my_t_lines, total - my_t_lines + 1);
        hs_assign("PGBUTTONDN", hs_genbuf[5]);
	sprintf(hs_genbuf[6], "%d", start);
	hs_assign("START", hs_genbuf[6]);
	hs_end();

//	printf("[信件总数: %d]", total);
//	printf("[您的信箱容量为: %dK] [当前已用: %dK]", getmailboxsize(currentuser.userlevel), getmailsize(currentuser.userid));
//	printf("<br>[<a href=bbspstmail>发送信件</a>]");
	http_quit();
	return 0;
}

int show_mail() {
	char file[STRLEN];
	char title2[80];
	struct fileheader x;
	int start, total;
	init_all();
	if (!loginok) http_fatal("请先登陆");
	modify_user_mode(RMAIL);
	strsncpy(file, getparm("file"), 32);
	start=atoi(getparm("start"));
	setmailfile(file, ".DIR");
	total=file_size(file)/sizeof(x);
	if(total<=0) http_fatal("错误的参数3");
	if (start > total) start = total;
	if (start <= 0) start = 1;
	if (get_record(file, &x, sizeof(x), start) == -1) {
		http_fatal("无效的索引文件");
	}
	setmailfile(file, x.filename);

	hs_init(10);
	hs_setfile("bbsmailcon.ptn");
	snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "%d", start);
	hs_assign("START", hs_genbuf[0]);
	hs_assign("FILE", x.filename);
	hs_assign("USERID", x.owner);
	hs_assign("TITLE", x.title);
	hs_assign("ARTICLE", file);
	snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]), "%d", total);
	hs_assign("TOTAL", hs_genbuf[1]);
	if (start == 1) 
		snprintf(hs_genbuf[2], sizeof(hs_genbuf[2]), 
			"<font color=ff0000>首页 上一页</font>");
	else
		snprintf(hs_genbuf[2], sizeof(hs_genbuf[2]), 
			"<a href=bbsmailcon?start=1>首页</a> <a href=bbsmailcon?start=%d>上一页</a>",
			start - 1);
	if (start == total)
		snprintf(hs_genbuf[3], sizeof(hs_genbuf[3]), 
			"<font color=ff0000>下一页 末页</font>");
	else
		snprintf(hs_genbuf[3], sizeof(hs_genbuf[3]), 
			"<a href=bbsmailcon?start=%d>下一页</a> <a href=bbsmailcon?start=%d>末页</a>",
			start + 1, total);
	hs_assign("PGBUTTONUP", hs_genbuf[2]);
	hs_assign("PGBUTTONDN", hs_genbuf[3]);
	hs_end();

	x.flag |= FILE_READ;
	setmailfile(file, ".DIR");
	substitute_record(file, &x, sizeof(x), start);
	strcpy(title2, x.title);
	if(strncmp(x.title, "Re:",3)) sprintf(title2, "Re: %s", x.title);
	title2[60]=0;
	http_quit();
	return 0;
}

int show_newmail() {
	FILE *fp;
	struct fileheader x;
	int total=0, total2=0;
	char dir[80];
	char buf1[5][255];
   	init_all();
	/* Henry: 刷新消息栏，防止因为信件已读导致消息栏不更新 */
	//printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
	if(!loginok) http_fatal("您尚未登录, 请先登录");
	modify_user_mode(RMAIL);
	setmailfile(dir, ".DIR");
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("目前您的信箱没有任何信件");
	/* by Henry: 2002.12.8 */
	if (count_new_mails() == 0) http_fatal("目前您的信箱没有任何新信件");
	hs_init(20);
	hs_setfile("bbsnewmail.ptn");
	hs_setloop("mainform");
      	while(1) {
		if(fread(&x, sizeof(x), 1, fp)<=0) break;
		total++;
		if(x.flag & FILE_READ) continue;
		sprintf(buf1[0], "%d", total);
		hs_assign("INDEX", buf1[0]);
//		ptr=strtok(x.owner, " (");
//		if(ptr==0) ptr=" ";
//		ptr=nohtml(ptr);
//		sprintf(buf1[2], "%s", userid_str(ptr));
		hs_assign("FLAG", "N");
		sprintf(buf1[2], "%s", userid_str(x.owner));
		hs_assign("SENDER", buf1[2]);
		sprintf(buf1[3], "%6.6s", Cdtime(atoi(x.filename+2)));
		hs_assign("DATE", buf1[3]);
                sprintf(buf1[4], "<a href=bbsmailcon?file=%s&start=%d>%s%46.46s</a>",
                        x.filename, total, strncmp("Re: ", x.title, 4)?
                        "★ ":"", void1(x.title));
		hs_assign("TITLE", buf1[4]);
		total2++;
		hs_doloop("mainform");
      	}
	hs_end();
	http_quit();
	return 0;
}

int write_mail() {
   	FILE *fp;
   	int i, filenum=0, total;
	char buf[512], path[STRLEN], *userid, *file;
	struct userec *urec;
	struct fileheader x;

   	init_all();
	if(!loginok) http_fatal("匆匆过客不能写信，请先登录");
	modify_user_mode(SMAIL);

	/* Henry: check mail perm 02.07.11 */
	if (!has_mail_perm(&currentuser)) http_fatal("您被取消了发信权");
	file = getparm("file");
	strlcpy(buf, getparm("filenum"), 5);
	userid = getparm("userid");

	urec = 0;
	if (strlen(file) != 0) {
		filenum = atoi(buf);
		setmailfile(path, ".DIR");
		total = file_size(path) / sizeof (x);
		if (total < 0) http_fatal("无法读取邮件，请与系统维护联系");
		if (total == 0) http_fatal("您的邮箱没有信件");
		if (filenum <= 0 || filenum > total) filenum = total;
		if (search_record(path, &x, sizeof(x), cmpfilename, file) != 0)
			urec = getuser(x.owner);
	}

	hs_init(10);
	hs_setfile("bbspstmail.ptn");
	if (urec && !strcasecmp(urec->userid, userid)) {	//回复
		if (strncmp(x.title, "Re: ", 4))
			snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "Re: %s", void1(x.title));
		else
			snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "%s", void1(x.title));
		hs_assign("TITLE", urec?hs_genbuf[0]:"");
	} else if (urec) {	//转寄
		snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "[转寄] %s", void1(x.title));
		hs_assign("TITLE", hs_genbuf[0]);
	} else
		hs_assign("TITLE", "");

	hs_assign("USERID", userid);
	char content[2048];
	int contlen = sizeof(content) / sizeof(char);

	if (urec) {
		int lines=0;
		setmailfile(path, x.filename);
		fp=fopen(path, "r");
		if(fp) {
			if (!strcasecmp(urec->userid, userid)) {
			for(i=0; i<4; i++) {
				if(fgets(buf, 500, fp)==0) break;
				while (buf[strlen(buf) - 1] == '\n')
					buf[strlen(buf) - 1] = 0;
				if (i == 0) snprintf(content, sizeof(content), 
					"\n\n【 在 %s 的来信中提到: 】\n", &buf[8]);
			}
				
			while(1) {
				if(fgets(buf, 500, fp)==0) break;
				if(!strncmp(buf, ": 【", 4)) continue;
				if(!strncmp(buf, ": : ", 4)) continue;
				if(!strncmp(buf, "--\n", 3)) break;
				if(buf[0]=='\n') continue;;
				if(!strcasestr(buf, "</textarea>")) {
					safe_strcat(content, ": ", 0, &contlen);
					safe_strcat(content, buf, 0, &contlen);
				}
				if(lines++>20) {
					safe_strcat(content, ": (以下引言省略 ......)", 0, &contlen);
					break;
				}
			}
			} else {
				content[0] = '\0';
				while (1) {
					if (fgets(buf, sizeof(buf), fp) == 0) break;
					safe_strcat(content, buf, 0, &contlen);
				}
			}
			fclose(fp);
		}
		hs_assign("CONTENT", content);
	} else {
		hs_assign("CONTENT", "");
	}
	char filenumbuf[10];
	snprintf(filenumbuf, sizeof(filenumbuf), "%d", filenum);
	hs_assign("FILENUM", filenumbuf);
	hs_end();
	http_quit();
	return 0;
}

int do_send_mail() {
	char userid[80], filename[80], title[TITLELEN + 2], title2[STRLEN], buf[80], *content;
	int i, sig, backup, re=0;
   	struct userec *u;
	init_all();
	if(!loginok) http_fatal("匆匆过客不能写信，请先登录");
	/* Henry: check mail perm */
	if (!has_mail_perm(&currentuser)) http_fatal("很抱歉，你没有发送信件的权利");
   	strsncpy(userid, getparm("userid"), 40);
   	strsncpy(title, getparm("title"), TITLELEN);
	backup=atoi(getparm("backup"));
	re=atoi(getparm("filenum"));
	if(strstr(userid, ";") || strstr(userid, "'"))
	{
		sprintf(buf, "%s %s %s attack from wwwmail:%s\n", Ctime(time(0)), currentuser.userid, fromhost, userid);
		file_append("wwwlog/wwwattack.log", buf);
	}
	if (!strstr(userid, "@")) {	//Henry: check userperm for local mail
		u=getuser(userid);
		if(u==0) http_fatal("错误的收信人帐号");
		strcpy(userid, u->userid);
		if (!(u->userlevel & PERM_READMAIL))
			http_fatal("使用者 %s 无法收信", userid);
		if ((u->userlevel & PERM_SUICIDE) && (!HAS_PERM(PERM_SYSOP)))
			http_fatal("[%s] 自杀中，无法收信", userid);
		if(is_maildeny(userid))
			http_fatal("[%s] 不想收到你的信", userid);
		if (getmailboxsize(u->userlevel) * 2 <
		    getmailsize(u->userid))
			http_fatal("使用者 %s 无法收信，信箱已满\n", userid);
	}

  	for(i=0; i<strlen(title); i++)
		if(title[i]<=27 && title[i]>=-1) title[i]=' ';
        sig=atoi(getparm("signature"));
        if (getparm("usesignature")[0] != 'Y') sig = 0;
        else if (atoi(getparm("randomsig"))) {
                srand((unsigned) time(NULL));
                i = count_signature();
                if (i) sig = rand() % i + 1;
        }
   	content=getparm("text");
   	if(title[0]==0)
      		strcpy(title, "没主题");
	sprintf(filename, "tmp/%d.tmp", getpid());
	file_append(filename, content);
	sprintf(title2, "{%s} %s", userid, title);
	title2[56]=0;
	post_mail(userid, title, filename, currentuser.userid, currentuser.username, fromhost, sig-1, re);
	if(backup)
		post_mail(currentuser.userid, title2, filename, currentuser.userid, currentuser.username, fromhost, sig-1, 0);
	unlink(filename);
	http_fatal("信件已寄给%s.<br>\n", userid);
	/*if(backup) printf("信件已经备份.<br>\n");
	printf("<a href='javascript:history.go(-2)'>返回</a>");*/
	http_quit();
	return 0;
}

int del_mail() {
	FILE *fp;
	struct fileheader f;
	char path[80], file[80], *id;
	int num=0;
	init_all();
	if(loginok == 0) http_fatal("您尚未登录");
	id=currentuser.userid;
	strsncpy(file, getparm("file"), 20);
	if(strncmp(file, "M.", 2) || strstr(file, "..")) http_fatal("错误的参数");
	sprintf(path, "mail/%c/%s/.DIR", mytoupper(id[0]), id);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("错误的参数2");
	while(1) {
		if(fread(&f, sizeof(f), 1, fp)<=0) break;
		num++;
		if(!strcmp(f.filename, file)) {
			fclose(fp);
			delete_record(path, sizeof(struct fileheader), num);
			http_fatal("信件已删除.<br><a href=bbsmail>返回所有信件列表</a>\n");
			http_term();
		}
	}
	fclose(fp);
	http_fatal("信件不存在, 无法删除");
	http_quit();
	return 0;
}
