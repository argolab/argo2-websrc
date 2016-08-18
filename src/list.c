#include "webbs.h"

int show_friends() {
	struct user_info user[MAXACTIVE];
	int i, total=0, f_num; 
	struct user_info *x;
	char buf[8][255];
	init_all();
	if (!loginok) http_fatal("尚未登陆");
	if (!HAS_PERM(PERM_LOGINOK))
		http_fatal("非注册用户无权使用本功能");
	modify_user_mode(FRIEND);

	hs_init(10);
	hs_setfile("bbsfriend.ptn");
	for(i=0; i<MAXACTIVE; i++) {
		x=&(shm_utmp->uinfo[i]);
		if(x->active==0) continue;
		if(x->invisible && !HAS_PERM(PERM_SEECLOAK) &&
			strcasecmp(x->userid, currentuser.userid)) continue;
		memcpy(&user[total], x, sizeof(struct user_info));
		total++;
	}
	qsort(user, total, sizeof(struct user_info), cmpuser);
	hs_setloop("mainform");
	for(i=0, f_num=0; i<total; i++) {
		int dt=(time(0)-user[i].idle_time)/60;
		if(!isfriend(user[i].userid)) continue;
		sprintf(buf[0], "%d", i+1);
		hs_assign("INDEX", buf[0]);
		sprintf(buf[1], "√");
		hs_assign("FLAG", buf[1]);
		sprintf(buf[2], "%s",
			userid_str(user[i].userid));
		hs_assign("ID", buf[2]);
		sprintf(buf[3], "<a href=bbsqry?userid=%s>%*s </a>",
			user[i].userid, NICKNAMELEN, user[i].username);
		hs_assign("NICK", buf[3]);
		sprintf(buf[4], "%20.20s",
			SHOW_IP((&(user[i])))?user[i].from:"某某IP");
		hs_assign("FROM", buf[4]);
		sprintf(buf[5], "<font class=c%d>%s</font>", 
			user[i].invisible ? 36 :
			(www_mode(&user[i])) ? 35 : 30, 
			ModeType(user[i].mode));
		hs_assign("MODE", buf[5]);
		sprintf(buf[6], "%s", dt?inttostr(dt):"&nbsp");
		hs_assign("IDLE", buf[6]);
		hs_doloop("mainform");
		f_num++;
	}
	if(f_num==0){
		hs_assign("INDEX", "");
		hs_assign("FLAG", "");
		hs_assign("ID", "");
		hs_assign("NICK", "");
		hs_assign("FROM", "");
		hs_assign("MODE", "");
		hs_assign("IDLE", "");
		hs_doloop("mainform");
	}
	hs_end();
	http_quit();
	return 0;
}

int show_online_user() {
	struct user_info user[MAXACTIVE];
	int my_t_lines;
	int i, j, start, total; 
	struct user_info *x;
	char buf[7][255];
	init_all();
	modify_user_mode(LUSERS);
	total = 0;
	hs_init(20);
	hs_setfile("bbsusr.ptn");
	hs_assign("BBSHOST", BBSHOST);
	hs_assign("BBSNAME", BBSNAME);
	for(i=0; i<MAXACTIVE; i++) {
		x=&(shm_utmp->uinfo[i]);
		if(x->active==0) continue;
		if(x->invisible && !HAS_PERM(PERM_SEECLOAK) &&
			strcasecmp(x->userid, currentuser.userid)) continue;
		memcpy(&user[total], x, sizeof(struct user_info));
		total++;
	}
	start=atoi(getparm("start"));
	my_t_lines=atoi(getparm("my_t_lines"));
	if(my_t_lines<10 || my_t_lines>40) my_t_lines=20;
	qsort(user, total, sizeof(struct user_info), cmpuser);
	if(start>total-my_t_lines + 1) start=total-my_t_lines + 1;
	if(start<1) start=1;
	i = start - 1; j = my_t_lines;
	hs_setloop("mainform");
	while (i < start - 1 + j && i < total) {
		if (!is_rejected(user[i].userid)) {
			int dt=(time(0)-user[i].idle_time)/60;
			sprintf(buf[0], "%d", i + my_t_lines - j + 1);
			hs_assign("INDEX", buf[0]);
			sprintf(buf[1], "%s", isfriend(user[i].userid) ?
				"√" : "&nbsp");
			hs_assign("FRIENDFLAG", buf[1]);
			hs_assign("ID", user[i].userid);
			hs_assign("NICK", nohtml(user[i].username));
			sprintf(buf[4], "%18.18s", SHOW_IP((&(user[i])))?user[i].from:"某某IP");
			hs_assign("IP", buf[4]);
			sprintf(buf[5], "<font class=c%d>%s</font>", 
				user[i].invisible ? 36 :
				(www_mode(&user[i])) ? 35 : 32, 
				ModeType(user[i].mode));
			hs_assign("ACTION", buf[5]);
			hs_assign("DELAY", dt?inttostr(dt):"&nbsp");
			hs_doloop("mainform");
		} else {
			j ++;
		}
		i ++;
	}
        sprintf(buf[0], "%d", total);
        hs_assign("TOTAL", buf[0]);
        sprintf(buf[1], "%d", my_t_lines);
        hs_assign("PERPAGE", buf[1]);
        sprintf(buf[2], "%d", (start - 1) / my_t_lines + 1);
        hs_assign("PAGE", buf[2]);
        sprintf(buf[3], "%d", (total - 1) / my_t_lines + 1);
        hs_assign("PGTOTAL", buf[3]);
        if (start == 1)
                strcpy(buf[4], "<font color=ff0000>首页 上一页</font>");
        else
                sprintf(buf[4], "<a href=bbsusr?start=1>首页</a> <a href=bbsusr?start=%d>上一页</a>",
                        start - my_t_lines);
        hs_assign("PGBUTTONUP", buf[4]);
        if (start >= total - my_t_lines + 1)
                strcpy(buf[5], "<font color=ff0000>下一页 末页</font>");
        else
                sprintf(buf[5], "<a href=bbsusr?start=%d>下一页</a> <a href=bbsusr?start=%d>末页</a>",
                        start + my_t_lines, total - my_t_lines + 1);
        hs_assign("PGBUTTONDN", buf[5]);
	hs_end();

	http_quit();
	return 0;
}

int show_all_user() {
	FILE *fp;
	char buf[32];
	char buf1[8][255];
	struct userec x;
	int pos[MAXUSERS], total;
	int p, i, start, my_t_lines;
	init_all();
	modify_user_mode(LAUSERS);
	total = 0;
	for(i=0; i<MAXUSERS; i++) {
		if(shm_ucache->userid[i][0]>32) {
			pos[total]=i;
			total++;
		}
	}
	start=atoi(getparm("start"));
	if (start > total - 19) start = total - 19;
	if (start < 1) start = 1;
	my_t_lines = atoi(getparm("my_t_lines"));
	if (my_t_lines < 10 || my_t_lines > 40) my_t_lines = 20;
	fp=fopen(".PASSWDS", "r");
	hs_init(20);
	hs_setfile("bbsalluser.ptn");
	sprintf(buf, "%d", total);
	hs_assign("SUM", buf);
	hs_setloop("mainform");
	for(i = start; i < start + 20; i++) {
		if(i > total) break;
		p=pos[i - 1];
		fseek(fp, sizeof(struct userec)*p, SEEK_SET);
		if(fread(&x, sizeof(x), 1, fp)<=0) break;
		sprintf(buf1[0], "%d", i);
		hs_assign("INDEX", buf1[0]);
//		sprintf(buf1[1], "%s", isfriend(x.userid)?
//			"<font color=green>√</font>" : "&nbsp");
		hs_assign("FRIENDFLAG", isfriend(x.userid)?
			"<font color=green>√</font>" : "&nbsp");
//		sprintf(buf1[2], "%s", userid_str(x.userid));
		hs_assign("ID", x.userid);
		hs_assign("NICK", nohtml(x.username)[0]?
			nohtml(x.username):"&nbsp");
//		sprintf(buf1[3], "%s", nohtml(x.username)[0]?
//			nohtml(x.username):"&nbsp");
		sprintf(buf1[4], "%d", x.numlogins);
		sprintf(buf1[5], "%d", x.numposts);
		hs_assign("LOGINS", buf1[4]);
		hs_assign("POSTS", buf1[5]);
		hs_assign("DATE", Cdtime(x.lastlogin));
		hs_doloop("mainform");
//		sprintf(buf1[6], "%s", Cdtime(x.lastlogin));
//		form_content(href, width, 3);
	}
        sprintf(buf1[0], "%d", total);
        hs_assign("TOTAL", buf1[0]);
        sprintf(buf1[1], "%d", my_t_lines);
        hs_assign("PERPAGE", buf1[1]);
        sprintf(buf1[2], "%d", (start - 1) / my_t_lines + 1);
        hs_assign("PAGE", buf1[2]);
        sprintf(buf1[3], "%d", (total - 1) / my_t_lines + 1);
        hs_assign("PGTOTAL", buf1[3]);
        if (start == 1)
                strcpy(buf1[4], "<font color=ff0000>首页 上一页</font>");
        else
                sprintf(buf1[4], "<a href=bbsalluser?start=1>首页</a> <a href=bbsalluser?start=%d>下一页</a>",
                        start - my_t_lines);
        hs_assign("PGBUTTONUP", buf1[4]);
        if (start >= total - my_t_lines + 1)
                strcpy(buf1[5], "<font color=ff0000>下一页 末页</font>");
        else
                sprintf(buf1[5], "<a href=bbsalluser?start=%d>下一页</a> <a href=bbsalluser?start=%d>末页</a>",
                        start + my_t_lines, total - my_t_lines + 1);
        hs_assign("PGBUTTONDN", buf1[5]);
	hs_end();
//	form_foot(start, total, 20, "bbsalluser");
	http_quit();
	return 0;
}

int show_bar() {
	init_all();
	printf("<font style='font-size:12px'>\n");
  	printf("<center>欢迎访问[%s]目前在线人数(www/all) [<font color=green>%d/%d</font>]", 
    		BBSNAME, count_www(), count_online());
	printf("</font>");
	http_quit();
	return 0;
}

int count_www() {
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].mode & WWW) total++;
	return total;
}

int top_ten() {
	FILE *fp;
	int m, i;
	char buf[8][255];
	char line[256], *ptr;

	init_all();
	fp = fopen("etc/posts/http.day", "r");
	if (fp == NULL) http_fatal("文件错误");

	hs_init(10);
	hs_setfile("bbstop10.ptn");
	hs_setloop("mainform");
	m = 0;
	while(m < 10) {
		m++;
		fgets(line, 256, fp);

		ptr = strtok(line, "\t");
		for (i = 0; i < 6; i++) {
			strcpy(buf[i], ptr);
			ptr = strtok(NULL, "\t");
		}

		sprintf(buf[6], "%d", m);
		hs_assign("INDEX", buf[6]);
		hs_assign("BOARD", buf[2]);
		sprintf(buf[7],
			"<a href='bbstcon?board=%s&file=%s'>%42.42s </a>",
			buf[2], buf[3], buf[1]);
		hs_assign("TITLE", buf[7]);
		hs_assign("ID", buf[0]);
		hs_assign("TOTAL", buf[5]);
		hs_doloop("mainform");

	}
	hs_end();
	fclose(fp);
	http_quit();
	return 0;
}


#define BRDNUM 15	/*how many boards to display*/

int board_top_ten() {
	FILE *fp;
	char buf[256], tmp[256], name[256], cname[256], cc[256];
	char buf1[5][255];
	int i, r;

	init_all();
        fp=fopen("0Announce/bbslist/board2", "r");
        if(fp==0) http_fatal("error 1");

	hs_init(10);
	hs_setfile("bbstopb10.ptn");
	hs_setloop("mainform");
	for(i=0; i<=BRDNUM; i++) {
		if(fgets(buf, 150, fp)==0) break;
		if(i==0) continue;
		r=sscanf(buf, "%s %s %s %s %s  %s", tmp, tmp, name, tmp, cname, cc);
		if(r==6) {
			sprintf(buf1[0], "%d", i);
			hs_assign("INDEX", buf1[0]);
			hs_assign("BOARD", name);
			hs_assign("BOARDNAME", cname);
			sprintf(buf1[3], "%s", cc);
			hs_assign("TOTAL", buf1[3]);
			hs_doloop("mainform");
		}
	}
	hs_end();
	fclose(fp);
	http_quit();
	return 0;
}

int show_menu() {
	char buf[10][256];
	char path[256];
	char mybrd[GOOD_BRC_NUM][80];
	FILE *fp;
	int i, mybrdnum=0;
	
  	init_all();
	hs_init(10);
	if(loginok){
		hs_setfile("bbsleft.ptn");	
		sprintf(buf[0], "<a href=bbsqry?userid=%s target=f3>%s</a><br>",
				currentuser.userid, currentuser.userid);
		hs_assign("USER", buf[0]);
		strcpy(buf[1], "未激活用户");
#ifndef NOEXP
                if(currentuser.userlevel & PERM_LOGINOK) strlcpy(buf[1], c_exp(countexp(&currentuser)), sizeof(buf[1]));
#else
		if(currentuser.userlevel & PERM_LOGINOK) strlcpy(buf[1], "普通帐号", sizeof(buf[1]));
#endif
                if (HAS_PERM(PERM_BOARDS)) strcpy(buf[1], "版主");
		if (HAS_PERM(PERM_XEMPT)) strcpy(buf[1], "永久帐号");
                if (HAS_PERM(PERM_SYSOP)) strcpy(buf[1], "本站站长");
		if (HAS_PERM(PERM_LOGINOK) && !HAS_PERM(PERM_POST)) strcpy(buf[1], "<font color=red>被封全站中</font>");
                if (HAS_PERM(PERM_BOARDS)) strcpy(buf[1], "版主");
		if (HAS_PERM(PERM_XEMPT)) strcpy(buf[1], "永久帐号");
                if (HAS_PERM(PERM_SYSOP)) strcpy(buf[1], "本站站长");
		if (HAS_PERM(PERM_LOGINOK) && !HAS_PERM(PERM_POST)) strcpy(buf[1], "<font color=red>坏孩子</font>");
                hs_assign("LEVEL", buf[1]);
 		
		if(!HAS_PERM(PERM_WELCOME))
			hs_assign("HAS_NO_AUTH", "");
		if(HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_OBOARDS))
			hs_assign("IS_ADMIN", "");
		setuserfile(path, ".goodbrd");
   		fp=fopen(path, "r");
   		if(fp) {
                	while(fgets(mybrd[mybrdnum], 80, fp))
                	{
                       		mybrd[mybrdnum][strlen(mybrd[mybrdnum])-1]='\0';
                       		mybrdnum++;
				/* Henry: prevent memory overaccess */
				if (mybrdnum > GOOD_BRC_NUM) break;
                	}
   			fclose(fp);
		}
		hs_setloop("mainform");
  		for(i=0; i<mybrdnum; i++) {
     			if(!board_read(mybrd[i]))
				sprintf(buf[2], "<a target=f3 href=bbsdoc?board=%s><b>%s</b></a><br>\n", mybrd[i], mybrd[i]);
			else
				sprintf(buf[2], "<a target=f3 href=bbsdoc?board=%s>%s</a><br>\n", mybrd[i], mybrd[i]);
			hs_assign("BOARD", buf[2]);
			hs_doloop("mainform");
		}
		if(mybrdnum == 0){
			hs_assign("BOARD", "无预定讨论区");
			hs_doloop("mainform");
		}
		if(HAS_PERM(PERM_CLOAK))
			hs_assign("CAN_CLOAK", "");
		//if (currentuser.userlevel & PERM_CLOAK)
		//	printf("<img src=/link.gif><a target=f3 href=bbscloak> 切换隐身</a><br>\n");
	}
	else{
		hs_setfile("bbsleft2.ptn");
	}
	hs_end();
	http_quit();
	return 0;
}

int show_notepad() {
	FILE *fp;
	char buf[256];
	struct title_t title;
	init_all();
	modify_user_mode(NOTEPAD);
	title.BM=title.num=0;
	title.depth=1;
	sprintf(buf, "留言版(%6.6s)", Cdtime(time(0)));
	
	printf("<html>"
	      "<head>"
	      "<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
	      "<title>%s</title>"
	      "<link href='templates/global.css' rel='stylesheet' type='text/css' />"
	      "<style>"
	      "body{margin-right:16px;}"
	      "</style>"
	      "</head"
      	      "<body>"
	      "<form name='form1' method='post' action=''>"
	      "<div id='head'>"
	      "<div id='location'>"
	      "<p><img src='images/location01.gif' alt='' align='absmiddle'/><a href='bbssec'>%s</a></p>"
	      "<p><img src='images/location03.gif' alt='' align='absmiddle' />%s</p>"
	      "</div>"
	      "</div>"
	      "<table border='0' cellpadding='0' cellspacing='0' class='table'>"
	      "<tr>"
	      "<td colspan='2' class='tb_head'>"
	      "<img src='images/table_ul05.gif' align='absmiddle' style='float:left; margin-left:0!important;margin-left:-3px;'/>"
	      "<div class='title'><img src='images/li01.gif' width='9' height='9' align='absmiddle'/>"
	      "查看留言版</div></td>"
	      "<td align='right' class='tb_head'><img src='images/table_ur.gif' alt=''/></td>"
	      "</tr>"
	      "<tr>" 
	      "<td class='tb_l'> </td>"
	      "<td class='tb_r'> </td>"
	      "</tr>"
	      "<tr>"
	      "<td class='tb_l'> </td>"
	      "<td class='border content2'>", 
		BBSNAME, BBSNAME, buf);
	fp=fopen("etc/notepad", "r");
	if(fp==0) {
		printf("今天的留言版为空");
	}
	else {
		while(1) {
			if(fgets(buf, 255, fp)==0) break;
			hprintf("%s", buf);
		}
	}
	fclose(fp);
	printf("</td>"
		"<td class='tb_r'> </td>"
		"</tr>"
		"<tr>"
		"<td class='tb_l'> </td>"
		"<td class='footer' >※ 来源:．逸仙时空 Yat-sen Channel bbs.sysu.edu.cn．</td>"
		"<td class='tb_r'> </td>"
		"</tr>"
		"<tr class='tb_bottom'>" 
		"<td><img src='images/table_bl.gif' alt=''/></td>"
		"<td></td>"
		"<td align='right'><img src='images/table_br.gif' alt=''/></td>"
		"</tr>"
		"</table>"
		"<div id='footer'>"
		"</div>"
		"</form>"
		"</body>"
		"</html>");
	http_quit();
	return 0;
}

int show_foot() {
	int dt=0, mail_total, mail_unread;
	char *id;
	char buf[5][256];
	
	init_all();
	id = "guest";
        hs_init(5);
	hs_setfile("bbsfoot.ptn");
	
	if(loginok) {
		id=currentuser.userid;
		dt=abs(time(0) - *(int*)(u_info->from+32))/60;
                u_info->idle_time=time(0);
        }
	sprintf(buf[0], "%16.16s", Ctime(time(0)));
	hs_assign("TIME", buf[0]);
	sprintf(buf[1], "<a href=bbsusr target=f3>%d</a>", count_online());
	hs_assign("ONLINE", buf[1]);
	sprintf(buf[2], "<a href=bbsqry?userid=%s target=f3>%s</a>", id, id);
	hs_assign("USER", buf[2]);
	if(loginok) {
		count_mails(id, &mail_total, &mail_unread);
		if(mail_unread==0) {
			sprintf(buf[3], "<a href=bbsmail target=f3>%d封</a>", mail_total);
		} else {
			sprintf(buf[3], "<a href=bbsmail target=f3>%d(新信<font color=red>%d</font>)</a>)",
					mail_total, mail_unread);
		}
		hs_assign("MAIL", buf[3]);
	}
	else{
		hs_assign("MAIL", "");
	}
	sprintf(buf[4], "%d小时%d分", dt/60, dt%60);
  	hs_assign("STAY", buf[4]);
	hs_end();
	http_quit();
	return 0;
}
