#include "webbs.h"

int 
do_send_msg (myuserid, mypid, touserid, topid, msg)
char *myuserid;
int mypid;
char *touserid;
int topid;
char *msg; {
        char msgbuf[256], buf3[256], *ptr;
        int i;
        for(i=0; i<strlen(msg); i++)
                if((0<msg[i] && msg[i]<=27 )|| msg[i]==-1) msg[i]=32;
        if (msg[i-1]=='\n') msg[i-1]=0;
        if(mypid<=0) return -1;
	ptr = Ctime(time(0));
        sprintf(msgbuf, "\033[0;1;44;36m%-12.12s\033[33m(\033[36m%-5.5s\033[33m):\033[37m%-54.54s\033[31m(^Z回)\033[m\033[%05dm\n",
                myuserid, ptr + 11, msg, mypid);
        sethomefile(buf3, touserid, "msgfile");
        file_append(buf3, msgbuf);
        sethomefile(buf3, touserid, "allmsgfile");
        file_append(buf3, msgbuf);
        sethomefile(buf3, touserid, "msgfile.me");
        file_append(buf3, msgbuf);
        sethomefile(buf3, myuserid, "msgfile.me");
        file_append(buf3, msgbuf);
        sethomefile(buf3, myuserid, "allmsgfile");
	ptr = Ctime(time(0));
        sprintf(msgbuf, "\033[1;32;40mTo \033[1;33;40m%-12.12s\033[m (%-5.5s):%-55.55s\n",
                touserid, ptr + 11, msg);
        file_append(buf3, msgbuf);
        if(topid<=0) return -1;
        kill(topid, SIGTTOU);
        kill(topid, SIGUSR2);  
        return 0;
}

int send_msg() {
	int i;
	int mode, destpid=0;
	char destid[20], msg[256];
	init_all();
	modify_user_mode(MSG);
	if(!loginok) http_fatal("匆匆过客不能发讯息, 请先登录！");
	if(!user_perm(&currentuser, PERM_LOGINOK)) http_fatal("你还没完成注册，不能发送信息！");
	if(!user_perm(&currentuser, PERM_MESSAGE)) http_fatal("你被封禁了发信息权，不能发送信息！");
	strsncpy(destid, getparm("destid"), 13);
	strsncpy(msg, getparm("msg"), 51);
	destpid=atoi(getparm("destpid"));
	if(destid[0]==0 || msg[0]==0) {
		/*
		strcpy(buf3, "<body onload='document.form0.msg.focus()'>");
		if(destid[0]==0) strcpy(buf3, "<body onload='document.form0.destid.focus()'>");
		printf("%s\n", buf3);
		printf("<form name=form0 action=bbssendmsg method=post>"
			"	  <input type=hidden name=destpid value=%d>"
			"送讯息给: <input name=destid maxlength=12 value='%s' size=12><br>"
			"讯息内容: <input name=msg maxlength=50 size=50 value='%s'><br>"
			"	  <input type=submit value=确认 width=6>"
			"</form>",
			destpid, destid, msg);
			*/
		hs_init(1);
		hs_setfile("bbssendmsg.ptn");
		hs_assign("USER", destid);
		hs_end();
		http_term();
	}
	if(u_info->invisible) http_fatal("隐身状态不能发送信息");
	if(getusernum(destid)<0) http_fatal("查无此人");
	for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].active)
		if(!strcasecmp(shm_utmp->uinfo[i].userid, destid)) {
			if(destpid!=0 && shm_utmp->uinfo[i].pid!=destpid) continue;
			destpid=shm_utmp->uinfo[i].pid;
			if(!(shm_utmp->uinfo[i].pager & ALLMSG_PAGER)) continue;
			if(shm_utmp->uinfo[i].invisible && !(currentuser.userlevel & PERM_SEECLOAK)) continue;
			mode=shm_utmp->uinfo[i].mode;
			if(mode==BBSNET || mode==PAGE) continue;
			if(mode==LOCKSCREEN) continue;
			if(!strcasecmp(destid, currentuser.userid))
				http_fatal("你不能给自己发讯息！");
			else {
				if(is_rejected(destid)) http_fatal("已经帮你送出消息");
				else 
				if(do_send_msg(currentuser.userid, u_info->pid, destid, destpid, msg)==0) 
					http_fatal("已经帮你送出消息");
				else
					http_fatal("发送消息失败");
			}
			http_term();
		}
	http_fatal("此人目前不在线或者无法接受消息");
	http_quit();
	return 0;
}

int get_msg() {
	static char buf[256], buf2[256]=".";
	char toid[13];
	int topid, i;
        init_all();
	
	printf("<html>");
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	printf("<style type=text/css>\n");
        printf("A {color: #0000FF}\n");
	printf("body{margin-right:16px;}");
        printf("</style>\n");
	printf("<body style='BACKGROUND-COLOR: #CDD2CF'>");
	if(loginok==0) {
		printf("<body style='BACKGROUND-COLOR: #CDD2CF'>");
		http_term();
	}
	setuserfile(buf, "wwwmsg");
	if(file_size(buf)>0) {
		int total;
		printf("<bgsound src=/msg.wav>\n");
		printf("<body onkeypress='checkrmsg(event.keyCode)' style='BACKGROUND-COLOR: #CDD2CF'>");
		total=file_size(buf)/129;
		get_record(buf, buf2, 129, 1);
		delete_record(buf, 129, 1);
		printf("<table width=100%%>\n");
		printf("<tr height='24'><td>");
		buf2[111]=0;
		hprintf(buf2);
		sscanf(buf2+12, "%s", toid);
		sscanf(buf2+122, "%d", &topid);
		printf("<td><font style='font-size:12pt;'><a target=f3 href=bbssendmsg?destid=%s&destpid=%d onclick=\"javascript:setTimeout('self.location=self.location', 0);\">[回讯息]</a> <a href=bbsgetmsg>[忽略]</a></font>", toid, topid);
		printf("</body></html>");
		http_term();
	}
	i = count_new_mails();
	if (i) {
		printf("<bgsound src=/msg.wav>\n");
		printf("<body onkeypress='checkrmsg(event.keyCode)' style='BACKGROUND-COLOR: #CDD2CF'>");
		printf("<table width=100%%>\n");
		printf("<tr><td><center><font color=#ee33cc>");
		printf("<font style='font-size:12pt;'><a target=f3 href=bbsnewmail>系统通知：您有 <font color=ff0000>%d</font> 封新信件，点击此处查看新信件列表</a></font>\n", i);
		printf("</font></center></td></tr></table>\n");
	}
	for(i=0;host[i][0];i++)
	{
		if(strstr(fromhost, host[i]))
			{
				printf("<script>setTimeout('self.location=self.location', 10000);</script>");
				break;
			}
	}
	printf("<script>setTimeout('self.location=self.location', 30000);</script>");
	printf("</body></html>");	
	http_quit();
	return 0;
}

int show_msg() {
	FILE *fp;
	char buf[512], path[512], msgid[512], msgid1[512], msgid2[512];
	int counter=0;
	init_all();
	if(!loginok) http_fatal("匆匆过客无法查看讯息, 请先登录");
	modify_user_mode(LOOKMSGS);
	setuserfile(path, "allmsgfile");
	fp=fopen(path, "r");
	if(fp==0) http_fatal("没有任何讯息");
	printf("<html>"
		"<head>"
		"<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
		"<title>%s</title>"
		"<link href='templates/global.css' rel='stylesheet' type='text/css' />"
		"<style>"
		  "body{margin-right:12px;}"
		  "</style>"
		  "</head>"
		  "<body>"
		  "<form name='form1' method='post' action=''>"
		  "<div id='head'>"
		  "<div id='location'>"
		  "<p><img src='images/location01.gif' alt='' align='absmiddle'/><a href='bbssec'>%s</a></p>"
		  "<p><img src='images/location03.gif' alt='' align='absmiddle' />查看所有信息</p>"
		  "</div>"
		  "</div>"
		  "<table border='0' cellpadding='0' cellspacing='0' class='table'>"
		  "<tr>"
		  "<td colspan='2' class='tb_head'>"
		  "<img src='images/table_ul05.gif' alt='' align='absmiddle' style='float:left; margin-left:0!important;margin-left:-3px;'/>"
		  "<div class='title'><img src='images/li01.gif' width='9' height='9' align='absmiddle'/>" 
		  "查看所有信息</div></td>"
		  "<td align='right' class='tb_head'><img src='images/table_ur.gif' alt=''/></td>"
		  "</tr>"
		  "<tr>" 
		  "<td class='tb_l'> </td>"
		  "<td class='tb_r'> </td>"
		  "</tr>"
		  "<tr>"
		  "<td class='tb_l'> </td>"
		  "<td class='border content2'>", 
		  BBSNAME, BBSNAME);
	while(1) {
		counter++;
		if (counter>500) {
			http_fatal("信息量太多，拒绝继续显示<br/><a href=bbsmailmsg>讯息寄回邮箱</a>");
			break;
		}	//add by Henry
		if(fgets(buf, 256, fp)<=0) break;
		sscanf(buf, "%s %s", msgid1, msgid2);
		if (!strcmp(msgid1, "\033[1;32;40mTo")) {
			hprintf("%s", buf);
		} else {
			sscanf(msgid1, "\033[0;1;44;36m%s", msgid);
			printf("<a href=\"bbssendmsg?destid=%s\">%s</a>", msgid, msgid);
			strcpy(buf, buf + 24);
			hprintf("      %s", buf);
		}
	}
	fclose(fp);
	printf("</td>" 
		"<td class='tb_r'></td>"
		"</tr>"
		"<tr>"
		"<td class='tb_l'> </td>"
		"<td class='footer' >"
		"<a onclick='return confirm(\"你真的要清除所有讯息吗?\")' href=bbsdelmsg>清除所有讯息</a> "
		"<a href=bbsmailmsg>讯息寄回邮箱</a>"
		"</td>"
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

int del_msg() {
	char path[80];
	init_all();
	if(!loginok) http_fatal("匆匆过客不能处理讯息, 请先登录");
	setuserfile(path, "allmsgfile");
	unlink(path);
	http_fatal("已删除所有讯息备份");
	http_quit();
	return 0;
}

int mail_msg() {
	char filename[80];
	init_all();
	if(!loginok) http_fatal("匆匆过客不能处理讯息，请先登录");
	setuserfile(filename, "allmsgfile");
	if (post_mail(currentuser.userid, "所有讯息备份", filename, 
	   currentuser.userid, currentuser.username, fromhost, -1, 0) == 0) {
		unlink(filename);
		http_fatal("讯息备份已经寄回您的信箱");
	} else {
		http_fatal("无法寄回讯息备份，请与系统维护联系");
	}
	http_quit();
	return 0;
}
