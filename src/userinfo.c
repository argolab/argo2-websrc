#include "webbs.h"


struct denyheader denyuser[256];
int denynum;

static struct override fff[MAXFRIENDS];
static int friendnum = 0;
static struct override bbb[MAXREJECTS];
static int badnum = 0;

int 
loaddenyuser (board)
char *board; {
	int fd;
	char path[STRLEN];
	setboardfile(path, board, ".DENYLIST");
	fd = open(path, O_RDONLY);
	if (fd == -1) return -1;
	denynum = file_size(path)/sizeof(struct denyheader);
	lseek(fd, 0, SEEK_SET);
	if (read(fd, denyuser, denynum * sizeof(struct denyheader)) < 0) 
		return -1;
	close(fd);
	return 0;
}
                        
int 
savedenyuser (board)
char *board; {
	int fd;
	int i, m;
	char path[STRLEN], *title;
	setboardfile(path, board, ".DENYLIST");
	fd = open(path, O_WRONLY);
	if (fd == -1) return -1;
	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, 0);
	for(i=0; i<denynum; i++) {
		title=denyuser[i].title;
		for(m=0; title[m]; m++) {
			if(title[m]<32 && title[m]>0) title[m]='.';
		}
		if (denyuser[i].blacklist[0]) {
			write(fd, &denyuser[i], sizeof(struct denyheader));
		}
	}
	close(fd);
	return 0;
}

int 
loadfriend (id)
char *id; {
	int fd;
        char file[STRLEN];
        if(!loginok) return -1;   
	sethomefile(file, id, "friends");
	fd = open(file, O_RDONLY);
	if (fd == -1) return -1;
        friendnum=read(fd, fff, sizeof(struct override) * MAXFRIENDS)
		/ sizeof(struct override);
	close(fd);
        return 0;
}
        
int 
isfriend (id)
char *id; {
        int n;
        loadfriend(currentuser.userid);
        for(n=0; n<friendnum; n++)
                if(!strcasecmp(id, fff[n].id)) return 1;
        return 0;
}

int 
loadbad (id)
char *id; {
	int fd;
        char file[256];
        if(!loginok) return -1;
	sethomefile(file, id, "rejects");
	fd = open(file, O_RDONLY);
	if (fd == -1) return -1;
        badnum = read(fd, bbb, sizeof(struct override) * MAXREJECTS)
		/ sizeof(struct override);
        close(fd);
        return 0;
}
 
int 
isbad (id)
char *id; {
        int n;
        loadbad(currentuser.userid);
        for(n=0; n<badnum; n++)
                if(!strcasecmp(id, bbb[n].id)) return 1;
        return 0;
}

int 
is_maildeny (id)
char *id; {
	char file[80];
	int item, i;
	struct override maildeny_list[200];

	sethomefile(file, id, "maildeny");
	item = get_num_records(file, sizeof(struct override));
	if (item > 200) item = 200;
	get_records(file, maildeny_list, sizeof(struct override), 1, item);
	for (i = 0; i < item; i++)
		if (!strcasecmp(maildeny_list[i].id, currentuser.userid))
			return 1;
	return 0;
}

int 
save_user_data (x)
struct userec *x; {
        FILE *fp;
        int n;   
        n=getusernum(x->userid);
        if(n<0 || n>1000000) return 0;
//      if (put_record(x, sizeof(struct userec), 1, PASSFILE) == -1)
//              http_fatal("�޷������û�");

        fp=fopen(".PASSWDS", "r+");
        if (fp == NULL) return -1; 
        fseek(fp, n*sizeof(struct userec), SEEK_SET);
        fwrite(x, sizeof(struct userec), 1, fp);
        fclose(fp);
        return 0;  
}

int 
count_life_value (urec)
struct userec *urec; {
        int i;
        i=(time(0) - urec->lastlogin)/60;
        if((urec->userlevel & PERM_XEMPT)||!strcasecmp(urec->userid, "guest"))
                return 999;
        if(urec->numlogins <= 3 && !(urec->userlevel & PERM_WELCOME))
                return (15*1440-i)/1440;
        if(!(urec->userlevel & PERM_LOGINOK))
                return (30*1440-i)/1440;
        if(urec->stay>1000000)
                return (365*1440-i)/1440;
	if (urec->userlevel & PERM_SUICIDE)
		return (3 * 1440 - i) / 1440;
        return (120*1440-i)/1440;
}
 
int 
countexp (x)
struct userec *x; {
        int tmp;
        if(!strcasecmp(x->userid, "guest")) return -9999;
        tmp=x->numposts + x->numlogins/3 + (time(0)-x->firstlogin)/86400 + x->stay/3600;
        if(tmp<0) tmp=0;
        return tmp;
}

int 
countperf (x)
struct userec *x; {
        int day, logins, posts;   
        if(!strcasecmp(x->userid, "guest")) return -9999;
        day=(time(0) - x->firstlogin)/86400+1;
        logins=x->numlogins;
        posts=x->numposts;
        if(day<=0 || logins<=0 || posts<0) return 0;
        return (10*posts/logins+10*logins/day);
}
                
int 
modify_mode (x, newmode)
struct user_info *x;
int newmode; {
        if (x==NULL) return -1;
        x->mode=newmode;
        return 0;
}

#ifndef NOEXP
char *
cperf (perf)
int perf; {
	if(perf < 0) 	return GLY_CPERF0;
	if(perf <= 5)	return GLY_CPERF1;
	if(perf <= 12)	return GLY_CPERF2;
	if(perf <= 35)	return GLY_CPERF3;
	if(perf <= 50)	return GLY_CPERF4;
	if(perf <= 90)	return GLY_CPERF5;
	if(perf <= 140)	return GLY_CPERF6;
	if(perf <= 200)	return GLY_CPERF7;
	if(perf <= 500)	return GLY_CPERF8;
	return "�����ˣ�";
}

char *
c_exp (exp)
int exp; {
        if(exp<0)       return GLY_CEXP0;
        if(exp<=100)    return GLY_CEXP1;
        if(exp<=450)    return GLY_CEXP2;
        if(exp<=850)    return GLY_CEXP3;
        if(exp<=1500)   return GLY_CEXP4;
        if(exp<=2500)   return GLY_CEXP5;  
        if(exp<=3000)   return GLY_CEXP6;  
        if(exp<=5000)   return GLY_CEXP7;
        if(exp<=10000)  return GLY_CEXP8;
        return "��������";
}
#endif

char *
horoscope (month, day)
int month;
int day; {
        int date=month*100+day;
        if(month<1 || month>12 || day<1 || day>31) return "����";
        if(date<121 || date>=1222) return "Ħ����";
        if(date<219) return "ˮƿ��";
        if(date<321) return "˫����";
        if(date<421) return "ĵ����";
        if(date<521) return "��ţ��";
        if(date<622) return "˫����";
        if(date<723) return "��з��";
        if(date<823) return "ʨ����";
        if(date<923) return "��Ů��";
        if(date<1024) return "�����";
        if(date<1123) return "��Ы��";
        if(date<1222) return "������";
        return NULL;
}


int query_user() {
	FILE *fp;
	char userid[14], filename[80], buf[512];
	struct userec *x;
	struct user_info *u;
	int i, tmp1, tmp2, num, clr;
	init_all();
	modify_user_mode(QUERY);
	strsncpy(userid, getparm("userid"), 13);
	if(userid[0]==0) {
		hs_init(1);
		hs_setfile("search_usr.ptn");
		hs_end();
		http_term();
	}
	x=getuser(userid);
	if(x==0) {
		http_fatal("�û� [%s] ������.", userid);
		http_term();
	}

	printf("<html>"
		"<head>"
		"<meta http-equiv=Content-Type content=text/html; charset=gb2312 />"
		"<title>%s</title>"
		"<link href=templates/global.css rel=stylesheet type=text/css />"
		"<style type=text/css>"
		" body{"
		" margin-right:16px;"
		" }"
		"</style>"
		"</head>"		  
		"<body>"
		"<form name=form1 method=post>"
		"<div id=head>"
		"<div id=location>"
		"<p><img src=images/location01.gif alt= align=absmiddle width=11 height=14/><a href=bbssec>%s</a></p>"
		"<p><img src=images/location03.gif alt= align=absmiddle/>��ѯ����</p>"
      		"</div>"				  
		"</div>"
		"<table border=0 cellpadding=0 cellspacing=0 class=table>"
		"<tr> "
		"<td colspan=2 class=tb_head><img src=images/table_ul06.gif alt= width=37 height=25 align=absmiddle class=tb_ul style=float:left /> "
		"<div style=margin-top:8px>��ѯ����</div></td>"
		"<td width=19 align=right valign=top class=tb_r><img src=images/table_ur.gif alt=/></td>"
		"</tr>"
		"<tr>" 
		"<td class=tb_l> </td>"
		"<td>"
		"<ul class=search>"
		"<li>"
		, BBSNAME, BBSNAME);
	
	sprintf(buf, "%s (%s) ����վ %d �Σ��������� %d ƪ", 
		x->userid, x->username, x->numlogins, x->numposts);
	hprintf("%s", buf);
	
	show_special(x->userid);
#ifdef DEF_COLOREDSEX
                clr=(x->gender == 'F') ? 35 : 36;
#else
                clr=32;
#endif

#ifdef DEF_S_HOROSCOPE
	hprintf("[\033[1;%dm%s\033[m]", clr, horoscope(x->birthmonth, x->birthday));
#endif
	search_ulist(&u, x->userid);
	printf("<br/>");
	hprintf("�ϴ��� [%s] �� [%s] ����վһ��", 
		Ctime(x->lastlogin), SHOW_IP(u)?x->lasthost:"ĳ̨����");
	printf("<br/>");
	count_mails(userid, &tmp1, &tmp2);
	hprintf("���䣺[\033[32m%s\033[37m]��", tmp2 ? "��":"  ");
#ifndef NOEXP
	hprintf("����ֵ��[\033[32m%d\033[37m](\033[33m%s\033[37m) ", countexp(x), c_exp(countexp(x)));
	hprintf("����ֵ��[\033[32m%d\033[37m](\033[33m%s\033[37m) ", countperf(x), cperf(countperf(x)));
#endif
	hprintf("��������[%d]��", count_life_value(x));
	printf("<br/>");
	num=0;
	for(i=0; i<MAXACTIVE; i++) {
		u=&(shm_utmp->uinfo[i]);
		if(!strcmp(u->userid, x->userid)) {
			if (u->active==0 || u->pid==0 ||(u->invisible && 
			    !HAS_PERM(PERM_SEECLOAK) &&
			    strcasecmp(u->userid, currentuser.userid))) 
				continue;
			num++;
			if(num==1) hprintf("Ŀǰ��վ��, ״̬����:\n");
			if (u->invisible) hprintf("[��]\033[36m");
			else if (www_mode(u)) hprintf("\033[35m");
			else hprintf("\033[32m");
//			if(u->invisible) hprintf("\033[36mC\033[37m");
			hprintf("%s\033[m ", ModeType(u->mode));
			printf("    ");
			if(num%5==0) printf("\n");
			printf("<br/>");
		}
	}
	if(num==0) {
		hprintf("Ŀǰ����վ��, �ϴ���վʱ�� [\033[1;32m%s\033[m] ", Ctime(x->lastlogout));
		printf("<br/>");
	}

	if (HAS_PERM(PERM_SYSOP)) {
		printf("��ʵ������%s\n<br/>", x->realname);
	}
	printf("</li>");
	sethomefile(filename, x->userid, "plans");
	fp=fopen(filename, "r");
	printf("<li><pre>");
	if(fp) {
		while(1) {
			if(fgets(buf, 256, fp)==0) break;
			hhprintf("%s", buf);
		}
		fclose(fp);
	} else {
		hprintf("\033[36mû�и���˵����\033[37m\n");
	}
	printf("</pre></li>"
		"</ul>"
		"</td>"
		"<td class=tb_r> </td>"
		"</tr>"
		"<tr class=tb_bottom>"
		"<td width=25><img src=images/table_bl.gif alt=/></td>"
		"<td width=1060></td>"
		"<td align=right><img src=images/table_br.gif alt=/></td>"
		"</tr>"
		"</table>"
	        "<li>");
	
	printf("<a href=bbspstmail?userid=%s&title=û����>д���ʺ� </a> ", x->userid);
	printf("<a href=bbssendmsg?destid=%s>����ѶϢ </a> ", x->userid);
	printf("<a href=bbsfadd?userid=%s>������� </a> ", x->userid);
	printf("<a href=bbsfdel?userid=%s>ɾ������ </a>", x->userid);
	printf("</li>"
		"</form>"
		"</body>"
		"</html>");
	http_quit();
	return 0;
}

int show_special(char *id2) {
        FILE *fp;
        char  id1[80], name[80];
        fp=fopen("etc/sysops", "r");
        if(fp==0) return -1;
        while(1) {
                id1[0]=0;
                name[0]=0;
                if(fscanf(fp, "%s %s", id1, name)<=0) break;
                if(!strcmp(id1, id2))
			hprintf(" \033[1;31m��\033[0;36m%s\033[1;31m��\033[m", name);
        }
        fclose(fp);
	return 0;
}

int change_nick() {
	int i;
	unsigned char nick[80];
	init_all();
	if(!loginok) http_fatal("�Ҵҹ����޷��ı��ǳ�");
	strsncpy(nick, getparm("nick"), 20);
#ifdef FILTER
	/* babydragon: �ؼ��ֹ��� */
	if(check_text("", nick) == 0)
		http_fatal("�����ǳ��к��в����ʵ�����, �޷�����");
#endif
	if(nick[0]==0) {
		hs_init(5);
		hs_setfile("bbsnick.ptn");
		hs_assign("USER", currentuser.userid);
		hs_assign("NICK", void1(u_info->username));
		hs_end();
		http_quit();
	}
	for(i=0; nick[i]; i++)
		if(nick[i]<32 || nick[i]==255) nick[i]=' ';
	strlcpy(u_info->username, nick, NICKNAMELEN + 1);
	http_fatal("��ʱ����ǳƳɹ�");
	return 0;
}

int change_signature() {
	int fd;
	char *ptr, path[256], buf[10000];
   	init_all();
	modify_user_mode(EDITUFILE);
	if(!loginok) http_fatal("�Ҵҹ��Ͳ�������ǩ���������ȵ�¼");
	setuserfile(path, "signatures");
	if(!strcasecmp(getparm("type"), "1")) save_signature(path);
	fd = open(path, O_RDWR | O_CREAT, 0644);
	if (fd == -1) http_fatal("�޷���ǩ�����ļ�");
	memset(buf, 0, sizeof(buf));
	read(fd, buf, sizeof(buf) - 1);
	ptr=strcasestr(buf, "<textarea>");
	if(ptr) ptr[0]=0;
	close(fd);

	hs_init(5);
	hs_setfile("bbssig.ptn");
	hs_assign("USER", currentuser.userid);
	hs_assign("CONTENT", void1(buf));
	hs_end();
	http_quit();
	return 0;
}

void save_signature(char *path) {
	char *buf;
	FILE *fp;
	
	buf=getparm("text");
#ifdef FILTER
	/* babydragon: �ؼ��ֹ��� */
	if(check_text("", buf) == 0)
		http_fatal("����ǩ�����к��в����ʵ�����, �޷�����");
#endif
	if( (fp=fopen(path, "w")) == NULL)
		http_fatal("open file error");
	fprintf(fp, "%s", buf);
	fclose(fp);
	http_fatal("ǩ�����޸ĳɹ���");
}

int cmpuser(a, b)
const void *a, *b;
{
        char id1[80], id2[80];
        sprintf(id1, "%d%s", !isfriend(((struct user_info *)a)->userid), ((struct user_info *)a)->userid);
        sprintf(id2, "%d%s", !isfriend(((struct user_info *)b)->userid), ((struct user_info *)b)->userid);
        return strcasecmp(id1, id2);
}


int change_passwd() {
	int type;
  	char pw1[20], pw2[20], pw3[20];
	init_all();
	modify_user_mode(EDITUFILE);
	if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
	type=atoi(getparm("type"));
	if(type==0) {
		hs_init(1);
		hs_setfile("bbspwd.ptn");
		hs_assign("USER", currentuser.userid);
		hs_end();
		http_quit();
	}
  	strsncpy(pw1, getparm("pw1"), 13);
  	strsncpy(pw2, getparm("pw2"), 13);
  	strsncpy(pw3, getparm("pw3"), 13);
  	if(strcmp(pw2, pw3)) http_fatal("�������벻��ͬ");
  	if(strlen(pw2)<2) http_fatal("������̫��");
//  	if(!checkpasswd(currentuser.passwd, pw1)) http_fatal("���벻��ȷ");
  	if(!checkpasswd2(pw1, &currentuser)) http_fatal("���벻��ȷ");
  	strcpy(currentuser.passwd, crypt_des(pw2, pw2));
  	save_user_data(&currentuser);
  	http_fatal("[%s] �����޸ĳɹ�.", currentuser.userid);
	return 0;
}

int add_bad() {
   	FILE *fp;
	char path[80], userid[80], exp[80];
	init_all();
   	if(!loginok) http_fatal("����δ��¼�����ȵ�¼");
	if (!HAS_PERM(PERM_LOGINOK)) http_fatal("δע���û��޷�ʹ�ñ�����");
	modify_user_mode(EDITUFILE);
	setuserfile(path, "rejects");
   	printf("<center>%s -- �������� [ʹ����: %s]<hr color=green>\n", BBSNAME, currentuser.userid);
	strsncpy(userid, getparm("userid"), 13);
	strsncpy(exp, getparm("exp"), 32);
	loadbad(currentuser.userid);
	if(userid[0]==0) {
		//if(userid[0]) printf("<font color=red>�����뻵��˵��</font>");
		printf("<form action=bbsbadd>\n");
		printf("������������Ļ����ʺ�: <input type=text name=userid value='%s'><br>\n",
			userid);
		printf("�������������˵�˵��: <input type=text name=exp value='%s'>\n", 
			exp);
		printf("<br><input type=submit value=ȷ��></form>\n");
		http_term();
	}
	if(!getuser(userid)) http_fatal("�����ʹ�����ʺ�");
	if(badnum>=MAXREJECTS) http_fatal("���Ļ��������Ѵﵽ����, ��������µĻ���");
   	if(isbad(userid)) http_fatal("�����Ѿ�����Ļ�����������");
	strcpy(bbb[badnum].id, getuser(userid)->userid);
	strcpy(bbb[badnum].exp, exp);
	badnum++;
   	if ((fp=fopen(path, "w")) == NULL) http_fatal("ϵͳ�����޷�������������");
   	fwrite(bbb, sizeof(struct override), badnum, fp);
   	fclose(fp);
   	printf("[%s]�Ѽ������Ļ�������.<br>\n <a href=bbsball>���ػ�������</a>", userid);
	http_quit();
	return 0;
}

int add_friend() {
   	FILE *fp;
	char path[80], userid[80], exp[80];
	init_all();
   	if(!loginok) http_fatal("����δ��¼�����ȵ�¼");
	if (!HAS_PERM(PERM_LOGINOK)) http_fatal("δע���û��޷�ʹ�ñ�����");
	modify_user_mode(EDITUFILE);
	setuserfile(path, "friends");
   	printf("<center>%s -- �������� [ʹ����: %s]<hr color=green>\n", BBSNAME, currentuser.userid);
	strsncpy(userid, getparm("userid"), 13);
	strsncpy(exp, getparm("exp"), 32);
	loadfriend(currentuser.userid);
	if(userid[0]==0) {
		//if(userid[0]) printf("<font color=red>���������˵��</font>");
		printf("<form action=bbsfadd>\n");
		printf("������������ĺ����ʺ�: <input type=text name=userid value='%s'><br>\n",
			userid);
		printf("�������������ѵ�˵��: <input type=text name=exp value='%s'>\n", 
			exp);
		printf("<br><input type=submit value=ȷ��></form>\n");
		http_term();
	}
	if(!getuser(userid)) http_fatal("�����ʹ�����ʺ�");
	if(friendnum>=199) http_fatal("���ĺ��������Ѵﵽ����, ��������µĺ���");
   	if(isfriend(userid)) http_fatal("�����Ѿ�����ĺ�����������");
	strcpy(fff[friendnum].id, getuser(userid)->userid);
	strcpy(fff[friendnum].exp, exp);
	friendnum++;
   	if ((fp = fopen(path, "w")) == NULL) http_fatal("ϵͳ����, ��������µĺ���");
   	fwrite(fff, sizeof(struct override), friendnum, fp);
   	fclose(fp);
   	printf("[%s]�Ѽ������ĺ�������.<br>\n <a href=bbsfall>���غ�������</a>", userid);
	http_quit();
	return 0;
}

int list_bad() {
	int i; 
	char buf[5][STRLEN];
	init_all();
   	if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
	if (!HAS_PERM(PERM_LOGINOK)) http_fatal("δע���û��޷�ʹ�ñ�����");
	modify_user_mode(EDITUFILE);
	loadbad(currentuser.userid);
	hs_init(10);
	hs_setfile("bbsball.ptn");
	if(badnum == 0){
		hs_assign("INDEX", "");
		hs_assign("ID", "");
		hs_assign("NOTE", "");
		hs_assign("DEL", "");
	}
	else{
		hs_setloop("mainform");
		for (i=0; i<badnum; i++) {
			sprintf(buf[0], "%d", i + 1);
			hs_assign("INDEX", buf[0]);
			sprintf(buf[1], "%s", userid_str(bbb[i].id));
			hs_assign("ID", buf[1]);
			sprintf(buf[2], "&nbsp;%s&nbsp;", nohtml(bbb[i].exp));
			hs_assign("NOTE", buf[2]);
			sprintf(buf[3], "[<a onclick='return confirm(\"ȷʵɾ����?\")' href=bbsbdel?userid=%s>ɾ��</a>]", 
				bbb[i].id);
			hs_assign("DEL", buf[3]);
			hs_doloop("mainform");
		}
	}
	hs_end();
	http_quit();
	return 0;
}

int list_friend() {
	int i; 
	char buf[5][STRLEN];
	init_all();
   	if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
	if (!HAS_PERM(PERM_LOGINOK)) http_fatal("δע���û��޷�ʹ�ñ�����");
	modify_user_mode(EDITUFILE);
	loadfriend(currentuser.userid);
//   	printf("�����趨�� %d λ����<br>", friendnum);
	hs_init(10);
	hs_setfile("bbsfall.ptn");
	if(friendnum == 0){
		hs_assign("INDEX", "");
		hs_assign("ID", "");
		hs_assign("NOTE", "");
		hs_assign("DEL", "");
	}
	else{
		hs_setloop("mainform");
		for (i=0; i<friendnum; i++) {
			sprintf(buf[0], "%d", i + 1);
			hs_assign("INDEX", buf[0]);
			sprintf(buf[1], "%s", userid_str(fff[i].id));
			hs_assign("ID", buf[1]);
			sprintf(buf[2], "&nbsp;%s&nbsp;", nohtml(fff[i].exp));
			hs_assign("NOTE", buf[2]);
			sprintf(buf[3], "[<a onclick='return confirm(\"ȷʵɾ����?\")' href=bbsfdel?userid=%s>ɾ��</a>]", 
			fff[i].id);
			hs_assign("DEL", buf[3]);
			hs_doloop("mainform");
		}
	}
   	hs_end();
	http_quit();
	return 0;
}

int del_friend() {
   	FILE *fp;
   	int i, total=0;
	char path[80], userid[80];
	struct override f[200];
	init_all();
	modify_user_mode(EDITUFILE);
   	if(!loginok) http_fatal("����δ��¼�����ȵ�¼");
	setuserfile(path, "friends");
   	printf("<center>%s -- �������� [ʹ����: %s]<hr color=green>\n", BBSNAME, currentuser.userid);
	strsncpy(userid, getparm("userid"), 13);
	if(userid[0]==0) {
		printf("<form action=bbsfdel>\n");
		printf("��������ɾ���ĺ����ʺ�: <input type=text><br>\n");
		printf("<input type=submit>\n");
		printf("</form>");
		http_term();
	}
   	loadfriend(currentuser.userid);
	if(friendnum<=0) http_fatal("��û���趨�κκ���");
   	if(!isfriend(userid)) http_fatal("���˱����Ͳ�����ĺ���������");
   	for(i=0; i<friendnum; i++) {
		if(strcasecmp(fff[i].id, userid)) {
			memcpy(&f[total], &fff[i], sizeof(struct override));
			total++;
		}
	}
   	fp=fopen(path, "w");
   	fwrite(f, sizeof(struct override), total, fp);
   	fclose(fp);
   	printf("[%s]�Ѵ����ĺ���������ɾ��.<br>\n <a href=bbsfall>���غ�������</a>", userid);
	http_quit();
	return 0;
}

int del_bad() {
   	FILE *fp;
   	int i, total=0;
	char path[80], userid[80];
	struct override f[200];
	init_all();
   	if(!loginok) http_fatal("����δ��¼�����ȵ�¼");
	setuserfile(path, "rejects");
   	printf("<center>%s -- �������� [ʹ����: %s]<hr color=green>\n", BBSNAME, currentuser.userid);
	strsncpy(userid, getparm("userid"), 13);
	if(userid[0]==0) {
		printf("<form action=bbsbdel>\n");
		printf("��������ɾ���Ļ����˺�: <input type=text><br>\n");
		printf("<input type=submit>\n");
		printf("</form>");
		http_term();
	}
   	loadbad(currentuser.userid);
	if(badnum<=0) http_fatal("��û���趨�κλ���");
   	if(!isbad(userid)) http_fatal("���˱����Ͳ�����Ļ���������");
   	for(i=0; i<badnum; i++) {
		if(strcasecmp(bbb[i].id, userid)) {
			memcpy(&f[total], &bbb[i], sizeof(struct override));
			total++;
		}
	}
   	fp=fopen(path, "w");
   	fwrite(f, sizeof(struct override), total, fp);
   	fclose(fp);
   	printf("[%s]�Ѵ����Ļ���������ɾ��.<br>\n <a href=bbsball>���ػ�������</a>", userid);
	http_quit();
	return 0;
}

int change_info() {
	int type;
  	init_all();
	if(!loginok) http_fatal("����δ��¼");
	modify_user_mode(EDITUFILE);
	type=atoi(getparm("type"));
	if(type!=0) {
		check_info();
		http_quit();
	}
 	
	printf("<html>"
		"<head>"
		"<meta http-equiv=Content-Type content=text/html; charset=gb2312 />"
		"<title>%s</title>"
		"<link href=templates/global.css rel=stylesheet type=text/css />"
		"<style type=text/css>"
		"body{margin-right:16px;}"
		"</style>"
		"</head>"
		"<body>"
		"<form name=form1 method=post action=bbsinfo?type=1>"
		"<div id=head>"
		"<div id=location>"
		"<p><img src=images/location01.gif alt= align=absmiddle/><a href=bbssec>%s</a></p>"
		"<p><img src=images/location03.gif alt= align=absmiddle/>��������</p>"
		"</div>"
		"</div>"
		"<table border=0 cellpadding=0 cellspacing=0 class=table>"
		"<tr>"
		"<td colspan=2 class=tb_head>"
		"<img src=images/table_ul01.gif alt= align=absmiddle class=tb_ul style=float:left />"
		"<div style=\"margin-top:8px\">�û���������</div></td>"
		"<td width=19 align=right valign=top class=tb_r><img src=images/table_ur.gif alt=/></td>"
		"</tr>"
		"<tr>" 
		"<td class=tb_l> </td>"
		"<td><ul class=search>"
		"<li>"
		, BBSNAME, BBSNAME);
  	printf("�����ʺ�: %s<br>\n", currentuser.userid);
  	printf("�����ǳ�: <input type=text name=nick value='%s' size=24 maxlength=%d><br>\n",
		currentuser.username, NICKNAMELEN + 2);
  	printf("�������: %d ƪ<br>\n", currentuser.numposts);
  	printf("�ż�����: %d ��<br>\n", currentuser.nummails);
  	printf("��վ����: %d ��<br>\n", currentuser.numlogins);
  	printf("��վʱ��: %d ����<br>\n", (int) currentuser.stay/60);
  	printf("��ʵ����: <input type=text name=realname value='%s' size=16 maxlength=16><br>\n",
	 	currentuser.realname);
  	printf("��ס��ַ: <input type=text name=address value='%s' size=40 maxlength=40><br>\n",
 		currentuser.address);
  	printf("�ʺŽ���: %s<br>", Ctime(currentuser.firstlogin));
  	printf("�������: %s<br>", Ctime(currentuser.lastlogin));
  	printf("��Դ��ַ: %s<br>", currentuser.lasthost);
  	printf("�����ʼ�: <input type=text name=email value='%s' size=32 maxlength=32><br>\n", 
		currentuser.email);
  	printf("��������: <input type=text name=year value=%d size=4 maxlength=4>��", 
		currentuser.birthyear+1900);
  	printf("<input type=text name=month value=%d size=2 maxlength=2>��", 
		currentuser.birthmonth);
  	printf("<input type=text name=day value=%d size=2 maxlength=2>��<br>\n", 
		currentuser.birthday);
  	printf("�û��Ա�: ");
    	printf("��<input type=radio value=M name=gender %s>", 
		currentuser.gender=='M' ? "checked" : "");
    	printf("Ů<input type=radio value=F name=gender %s><br>",
		currentuser.gender=='F' ? "checked" : "");
  	printf("</li>\n");
	printf("<li>"
		"<input type='image' src=images/confirm.gif class='confirm' width=54 height=24 border='0' />"
		"<a href=javascript:form1.reset()><img src=images/reset.gif alt=���� width=59 height=24 hspace=10 border=0/></a>"
		"</li>"
		"</ul></td>"
		"<td class=tb_r> </td>"
		"</tr>"
		"<tr class=tb_bottom> "
		"<td width=24><img src=images/table_bl.gif alt=/></td>"
		"<td width=1048></td>"
		"<td align=right><img src=images/table_br.gif alt=/></td>"
		"</tr>"
		"</table>"
		"</form>"
		"</body>"
		"</html>");
	return 0;
}

int check_info() {
  	int m;
  	char buf[256];
    	strsncpy(buf, getparm("nick"), 30);
    	for(m=0; m<strlen(buf); m++) if((buf[m]<32 && buf[m]>0) || buf[m]==-1) buf[m]=' ';
    	if(strlen(buf)>1) {
		strlcpy(currentuser.username, buf, NICKNAMELEN + 1);
	} else {
		http_fatal("����: �ǳ�̫��!<br>\n");
	}
    	strsncpy(buf, getparm("realname"), 9);
    	if(strlen(buf)>1) {
		strcpy(currentuser.realname, buf); 
	} else {
		http_fatal("����: ��ʵ����̫��!<br>\n");
	}
    	strsncpy(buf, getparm("address"), 40);
    	if(strlen(buf)>8) {
		strcpy(currentuser.address, buf);
	} else {
		http_fatal("����: ��ס��ַ̫��!<br>\n");
	}
    	strsncpy(buf, getparm("email"), 32);
   	if(strlen(buf)>8 && strchr(buf, '@')) {
		strcpy(currentuser.email, buf);
	} else {
		http_fatal("����: email��ַ���Ϸ�!<br>\n");
	}
    	strsncpy(buf, getparm("year"), 5);
    	if(atoi(buf)>1910 && atoi(buf)<1998) {
		currentuser.birthyear=atoi(buf)-1900;
	} else {
		http_fatal("����: ����ĳ������!<br>\n");
	}
    	strsncpy(buf, getparm("month"), 3);
    	if(atoi(buf)>0 && atoi(buf)<=12) {
		currentuser.birthmonth=atoi(buf);
	} else {
		http_fatal("����: ����ĳ����·�!<br>\n");
	}
    	strsncpy(buf, getparm("day"), 3);
    	if(atoi(buf)>0 && atoi(buf)<=31) {
		currentuser.birthday=atoi(buf);
	} else {
		http_fatal("����: ����ĳ�������!<br>\n");
	}
    	strsncpy(buf, getparm("gender"), 2);
    	if(!strcasecmp(buf, "F")) currentuser.gender='F';
    	if(!strcasecmp(buf, "M")) currentuser.gender='M';
    	save_user_data(&currentuser);
    	http_fatal("[%s] ���������޸ĳɹ�.", currentuser.userid);
	return 0;
}

int change_plan() {
	int fd;
	char *ptr, plan[256], buf[10000];
   	init_all();
	modify_user_mode(EDITUFILE);
	if(!loginok) http_fatal("�Ҵҹ��Ͳ�������˵���������ȵ�¼");
	setuserfile(plan, "plans");
	if(!strcasecmp(getparm("type"), "update")) save_plan(plan);
	fd = open(plan, O_RDWR | O_CREAT, 0644);
	if (fd == -1) http_fatal("�޷���˵�����ļ�");
	read(fd, buf, sizeof(buf) - 1);
	ptr=strcasestr(buf, "<textarea>");
	if(ptr) ptr[0]=0;
	close(fd);

	hs_init(2);
	hs_setfile("bbsplan.ptn");
   	hs_assign("USER", currentuser.userid);
	hs_assign("CONTENT", void1(buf));
	hs_end();
	http_quit();
	return 0;
}

int save_plan(char *plan) {
	int fd;
	char buf[10000];
	fd = open(plan, O_WRONLY);
	if (fd == -1) http_fatal("�޷���˵�����ļ�");
	truncate(plan, 0);
	strlcpy(buf, getparm("text"), sizeof(buf));
#ifdef FILTER
	/* babydragon: �ؼ��ֹ��� */
	if(check_text("", buf) == 0)
		http_fatal("���ĸ���˵���к��в����ʵ�����, �޷��޸�");
#endif
	/*
	while(buf[i])
	{
		if (buf[i]==13&&buf[i+1]=='\n')
			buf[i]=' ';
		i++;
	}
*/
	if (write(fd, buf, strlen(buf)) >= 0)
		http_fatal("����˵�����޸ĳɹ���");
	else
		http_fatal("����˵�����޸�ʧ��");
	close(fd);
	http_term();
	return 0;
}

int change_cloak() {
	init_all();
	if(!loginok) http_fatal("�Ҵҹ��Ͳ��ܽ��д˲���, ���ȵ�¼");
	if(!(currentuser.userlevel & PERM_CLOAK)) http_fatal("����Ĳ���");
	if(u_info->invisible) {
		u_info->invisible=0;
		http_fatal("����״̬�Ѿ�ֹͣ��.");
	} else {
		u_info->invisible=1;
		http_fatal("����״̬�Ѿ���ʼ��.");
	}
	http_quit();
	return 0;
}

char *defines[] = {
        "�������ر�ʱ���ú��Ѻ���",     /* DEF_FRIENDCALL */
        "���������˵�ѶϢ",             /* DEF_ALLMSG */
        "���ܺ��ѵ�ѶϢ",               /* DEF_FRIENDMSG */
        "�յ�ѶϢ��������",             /* DEF_SOUNDMSG */
        "ʹ�ò�ɫ",                     /* DEF_COLOR */
        "��ʾ�����",                 /* DEF_ACBOARD */
        "��ʾѡ����ѶϢ��",             /* DEF_ENDLINE */
        "�༭ʱ��ʾ״̬��",             /* DEF_EDITMSG */
        "ѶϢ������һ��/����ģʽ",      /* DEF_NOTMSGFRIEND */
        "ѡ������һ��/����ģʽ",        /* DEF_NORMALSCR */
        "��������ͼ�� New ��ʾ",        /* DEF_NEWPOST */
        "�Ķ������Ƿ�ʹ���ƾ�ѡ��",     /* DEF_CIRCLE */
        "�Ķ������α�ͣ춵�һƪδ��",   /* DEF_FIRSTNEW */
        "��վʱ��ʾ��������",           /* DEF_LOGFRIEND */
        "������վ֪ͨ",                 /* DEF_LOGINFROM */
        "�ۿ����԰漰����¼",           /* DEF_NOTEPAD*/
        "��Ҫ�ͳ���վ֪ͨ������",       /* DEF_NOLOGINSEND */
        "����ʽ����",                   /* DEF_THESIS */
        "�յ�ѶϢ�Ⱥ��Ӧ�����",       /* DEF_MSGGETKEY */
        "���ٽ�վ",                     /* DEF_QUICKLOGIN */
        "����Ⱥ����Ϣ",                 /* DEF_FRIENDWALL */      /* Rewrite by cancel */
        "ʹ������ǩ����",               /* DEF_RANDSIGN */
        "��վʱѯ�ʼĻ�����ѶϢ",       /* DEF_MAILMSG */
        "��������ɾ��",                 /* DEF_DELDBLCHAR */
        "�Զ��Ű���Ԥ��Ϊ 78 ��",     /* DEF_AUTOWRAP */
        "ʹ��GB���Ķ�",                 /* DEF_USEGB KCN 99.09.03 */
        "�������Լ��� IP",              /* DEF_NOTHIDEIP */
        "�Ժ�����ʾIP",                 /* DEF_FRIENDSHOWIP cancel 01.09.16 */
        "��ֹ����ANSI��չָ��",         /* DEF_NOANSI */
        "����ʾ�ײ�������Ϣ",           /* DEF_NOENDLINE */
        NULL
};

int change_parm() {
	int i, perm=1, type;
	init_all();
	type=atoi(getparm("type"));
	if (!loginok) http_fatal("�Ҵҹ��Ͳ����趨����");
	if (type) {
		save_parm();
		return 0;
	}
	printf("<html>"
		"<head>"
		"<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
		"<title>%s</title>"
		"<link href='templates/global.css' rel='stylesheet' type='text/css' />"
		"<style type='text/css'>"
		"body{margin-right:16px;}"
		"</style>"
		"</head>"		
		"<body>"
		"<form name='form1' method='post' action='bbsparm?type=1'>"
		"<div id='head'>"
		"<div id='location'>"
		"<p><img src='images/location01.gif' alt='' align='absmiddle'/><a href='bbssec'>%s</a></p>"
		"<p><img src='images/location03.gif' alt='' align='absmiddle' />�޸ĸ��˲���</p>"
		"</div>"
		"</div>"
		"<table border='0' cellpadding='0' cellspacing='0' class='table'>"
		"<tr>"
		"<td colspan='2' class='tb_head'>"
		"<img src='images/table_ul06.gif' alt='' align='absmiddle' class='tb_ul' style='float:left' />"
		"<div style='margin-top:8px'> ���˲����޸�</div> </td>"
		"<td align='right' valign='top' class='tb_head'><img src='images/table_ur.gif' alt=''/></td>"
		"</tr>"
		"<tr>"
		"<td height='37' colspan='2' class='td9'>ע��, ���²���������telnet��ʽ�²�������</td>"
		"<td class='tb_r td8'> </td>"
		"</tr>"
		"<tr>" 
		"<td colspan='2' class='td9' id='myboard_content'>",
		BBSNAME, BBSNAME);
	printf("<table>");
	for(i=0; defines[i]; i++) {
		char *ptr="";
		if(i%2==0) printf("<tr>");
		if(currentuser.userdefine & perm) ptr=" checked";
		printf("<td><input type=checkbox class=checkbox name=perm%d%s>%s</td>", i, ptr, defines[i]);
		perm=perm*2;
		if(i%2 == 1)
			printf("</tr>");
	}
	if(i%2 == 0)
		printf("<td></td></tr>");
	printf("</table>");
	printf("</td>"
		"<td class='tb_r td8'> </td>"
		"</tr>"
		"<tr>"
		"<td height='40' class='tb_head_left'> </td>"
		"<td colspan='2' class='tb_head_right'>"
		"<a href='javascript:form1.submit()'>"
		"<img src='images/confirm.gif' alt='ȷ��' hspace='2px' border='0'/></a>"
		"<a href='javascript:form1.reset()'>"
		"<img src='images/reset.gif' alt='����' border='0'/></a>"
		"</td>"
		"</tr>"
		"<tr class='tb_bottom'>"
		"<td><img src='images/table_bl.gif' alt=''/></td>"
		"<td width='563'></td>"
		"<td align='right'><img src='images/table_br.gif' alt=''/></td>"
		"</tr>"
		"</table>"
		"</form>"
		"</body>"
		"</html>");
	http_quit();
	return 0;
}

int save_parm() {
	int i, perm=1, def=0;
	char var[STRLEN];
	for(i=0; i<32; i++) {
		sprintf(var, "perm%d", i);
		if (strlen(getparm(var)) == 2) def += perm;
		perm = perm * 2;
	}
	currentuser.userdefine = def;
	if (save_user_data(&currentuser) == -1) return -1;
	http_fatal("���˲������óɹ�.<br><a href=bbsparm>���ظ��˲�������ѡ��</a>");
	return 0;
}


int config_interface() {
	FILE *fp;
	char *ptr, path[256], buf[256], buf1[256], buf2[256];
	int t_lines=20, link_mode=0, def_mode=0, type;
	init_all();
	if(!loginok) http_fatal("�Ҵҹ��Ͳ��ܶ��ƽ���");
	modify_user_mode(USERDEF);
	setuserfile(path, ".mywww");
	fp=fopen(path, "r");
	if(fp) {
		while(1) {
			if(fgets(buf, 80, fp)==0) break;
			if(sscanf(buf, "%80s %80s", buf1, buf2)!=2) continue;
			if(!strcmp(buf1, "t_lines")) t_lines=atoi(buf2);
			if(!strcmp(buf1, "link_mode")) link_mode=atoi(buf2);
			if(!strcmp(buf1, "def_mode")) def_mode=atoi(buf2);
		}
		fclose(fp);
	}
	type=atoi(getparm("type"));
	ptr=getparm("t_lines");
	if(ptr[0]) t_lines=atoi(ptr);
	ptr=getparm("link_mode");
	if(ptr[0]) link_mode=atoi(ptr);
        ptr=getparm("def_mode");
        if(ptr[0]) def_mode=atoi(ptr);
	if(type>0) return save_if_set(path, t_lines, link_mode, def_mode);
	if(t_lines<10 || t_lines>40) t_lines=20;
	if(link_mode<0 || link_mode>1) link_mode=0;
	
	hs_init(5);
	hs_setfile("bbsmywww.ptn");
	hs_assign("USER", currentuser.userid);
	sprintf(buf, "%d", t_lines);
	hs_assign("T_LINE", buf);
	sprintf(buf1, "%d", link_mode);
	hs_assign("LINK_MODE", buf1);
	sprintf(buf2, "%d", def_mode);
	hs_assign("DEF_MODE", buf2);
	hs_end();
	http_quit();
	return 0;
}

int save_if_set(char *path, int t_lines, int link_mode, int def_mode)
{
	FILE *fp;
	char buf[80];
	if(t_lines<10 || t_lines>40) http_fatal("���������");
	if(link_mode<0 || link_mode>1) http_fatal("���������ʶ�����");
	if(def_mode<0 || def_mode>1) http_fatal("�����ȱʡģʽ");
	fp=fopen(path, "w");
	if (fp == NULL) return -1;
	fprintf(fp, "t_lines %d\n", t_lines);
	fprintf(fp, "link_mode %d\n", link_mode);
	fprintf(fp, "def_mode %d\n", def_mode);
	fclose(fp);
	sprintf(buf, "%d", t_lines);
	setcookie("my_t_lines", buf);
	sprintf(buf, "%d", link_mode);
	setcookie("my_link_mode", buf);
        sprintf(buf, "%d", def_mode);
        setcookie("my_def_mode", buf);
	printf("WWW���Ʋ����趨�ɹ�.<br>\n");
	printf("[<a href='javascript:history.go(-2)'>����</a>]");
	return 0;
}

struct boardheader data[MAXBOARD];
char mybrd[GOOD_BRC_NUM][80];
int mybrdnum=0;

int list_favorbrd() {
	int total=0, i, type;
	char path[200];
   	FILE *fp;
	init_all();
	if(!loginok) http_fatal("��δ��¼���߳�ʱ");
	modify_user_mode(ZAP);

	type=atoi(getparm("type"));
	if(type!=0) {
		favorbrd_submit();
		http_term();
	}
	setuserfile(path, ".goodbrd");
   	fp=fopen(path, "r");
   	if(fp) {	
		while(fgets(mybrd[mybrdnum], 80, fp))
		{
			mybrd[mybrdnum][strlen(mybrd[mybrdnum])-1]='\0';
			mybrdnum++;
		}
		//mybrdnum--;
//		mybrdnum=fread(mybrd, sizeof(mybrd[0]), GOOD_BRC_NUM, fp);
  		fclose(fp);
	}
	printf("<html>"
		"<head>"
		"<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
		"<title>%s</title>"
		"<link href='templates/global.css' rel='stylesheet' type='text/css' />"
		"<style type='text/css'>"
		"body{margin-right:16px; }"
		"</style>"
		"</head>"
		"<body>"
		"<form action='bbsmybrd?type=1' method='post' name='form1'>"
		"<div id='head'>"
		"<div id='location'>"
		"<p><img src='images/location01.gif' alt='' align='absmiddle'/><a href='bbssec'>%s</a></p>"
		"<p><img src='images/location03.gif' alt='' align='absmiddle' />����Ԥ������������</p>"
		"</div>"
		"</div>"
		"<table border='0' cellpadding='0' cellspacing='0' class='table'>"
		"<tr>"
		"<td colspan='2' class='tb_head'>"
		"<img src='images/table_ul06.gif' alt='' align='absmiddle' class='tb_ul' style='float:left' />"
		"<div style='margin-top:8px'> ����Ԥ������������</div> </td>"
		"<td align='right' valign='top' class='tb_head'><img src='images/table_ur.gif' alt=''/></td>"
		"</tr>"
		"<tr>"
		"<td height='37' colspan='2' class='td9'>��ĿǰԤ����<strong>%d</strong>��������������Ԥ�� <strong>50</strong> ��</td>"
		"<td class='tb_r td8'> </td>"
		"</tr>"
		"<tr>"
		"<td colspan='2' class='td9' id='myboard_content'>", 
		BBSNAME, BBSNAME, mybrdnum);
	printf("<input type=hidden name=confirm1 value=1>\n");
	printf("<table class=body>\n");
	for(i=0; i<MAXBOARD; i++) {
		if(has_read_perm(&currentuser, shm_bcache->bcache[i].filename)) {
			memcpy(&data[total], &(shm_bcache->bcache[i]), sizeof(struct boardheader));
			total++;
		}
	}
	qsort(data, total, sizeof(struct boardheader), cmpboard);
	for(i=0; i<total; i++) {
		char *buf3="";
		if(ismybrd(data[i].filename) != -1) buf3=" checked";
		if(i%3==0) printf("\n<tr>");
		printf("<td class=body%d><input class=checkbox type=checkbox name=%s %s><a href=bbsdoc?board=%s>%s(%s)</a>", 
			(i/3)%2+1, data[i].filename, buf3,data[i].filename, data[i].filename, data[i].title+11);
	}
	printf("</table>\n");
	printf("</td>"
		"<td class='tb_r td8'> </td>"
		"</tr>"
		"<tr>"
		"<td height='40' class='tb_head_left'> </td>"
		"<td colspan='2' class='tb_head_right'>"
		"<a href='javascript:form1.submit()'><img src='images/confirm.gif' alt='ȷ��' hspace='2px' border='0'/></a>"
		"<a href='javascript:form1.reset()'><img src='images/reset.gif' alt='����' border='0'/></a>"
		"</td>"
		"</tr>"
		"<tr class='tb_bottom'>"
		"<td><img src='images/table_bl.gif' alt=''/></td>"
		"<td width='563'></td>"
		"<td align='right'><img src='images/table_br.gif' alt=''/></td>"
		"</tr>"
		"</table>"
		"</form>"
		"</body>"
		"</html>");
	http_quit();
	return 0;
}

int favorbrd_submit() {
        int i;
        char buf1[200];
        FILE *fp;
	int mybrdnum=0;
        if(!strcmp(getparm("confirm1"), "")) http_fatal("��������");
        for(i=0; i<parm_num; i++) {
                if(!strcasecmp(parm_val[i], "on")) {
                        if(mybrdnum>=GOOD_BRC_NUM) http_fatal("����ͼԤ������%d��������", GOOD_BRC_NUM);
                        if(!has_read_perm(&currentuser, parm_name[i])) {
                                printf("����: �޷�Ԥ��'%s'������<br>\n", nohtml(parm_name[i]));
                                continue;
                        }
                        strsncpy(mybrd[mybrdnum], parm_name[i], 80);
                        mybrdnum++;
                }
	}
	setuserfile(buf1, ".goodbrd");
        fp=fopen(buf1, "w");
	if (fp == NULL) http_fatal("�������趨ʧ��");
	for(i=0; i<mybrdnum; i++) {
		strcat(mybrd[i], "\n");
		fputs(mybrd[i], fp);
	}
   //     fwrite(mybrd, 80, mybrdnum, fp);
        fclose(fp);
        printf("<script>top.f2.location='bbsleft'</script>�޸�Ԥ���������ɹ���������һ��Ԥ����%d��������:<hr>\n", mybrdnum);
        printf("[<a href='javascript:history.go(-2)'>����</a>]");
	return 0;
}


int ismybrd(char *board) {
	int n;
	for(n=0; n<mybrdnum; n++) 
		if(!strcasecmp(mybrd[n], board)) return n;
	return -1;
}

int add_favorbrd() {
	FILE *fp;
	char file[200], board[200];
	int i;
	init_all();
	strsncpy(board, getparm("board"), 32);
	if(!loginok) http_fatal("��ʱ��δ��¼��������login");
	setuserfile(file, ".goodbrd");
	fp=fopen(file, "r");
	if(fp) {
                while(fgets(mybrd[mybrdnum], 80, fp))
                {
                        mybrd[mybrdnum][strlen(mybrd[mybrdnum])-1]='\0';
                        mybrdnum++;
                }
                fclose(fp);
	}
	if (mybrdnum >= GOOD_BRC_NUM) 
		http_fatal("��Ԥ����������Ŀ�Ѵ����ޣ���������Ԥ��");
	if (ismybrd(board) >= 0) http_fatal("���Ѿ�Ԥ�������������");
	if (!has_read_perm(&currentuser, board)) http_fatal("��������������");
	strcpy(mybrd[mybrdnum], board);
	fp=fopen(file, "w");
	if (fp == 0) http_fatal("Ԥ��������ʧ�ܣ�����ϵͳά����ϵ");
	for(i=0; i<=mybrdnum; i++) {
                strcat(mybrd[i], "\n");
                fputs(mybrd[i], fp);
        }
//	fwrite(mybrd, 80, mybrdnum+1, fp);
	fclose(fp);
	printf("<script>top.f2.location='bbsleft'</script>\n");
	printf("Ԥ���������ɹ�<br><a href='javascript:history.go(-1)'>���ٷ���</a>");
	http_quit();
	return 0;
}

int ranklist() {
	int fd;
	struct userec x;
	int logins=0, posts=0, stays=0, lifes=0, total=0;
	init_all();
	if(!loginok) http_fatal("�Ҵҹ��Ͳ���������");
	fd = open(PASSFILE, O_RDONLY);
	while(1) {
		if (read(fd, &x, sizeof(x)) <= 0) break;
		if(x.userid[0]<'A') continue;
		if(x.userlevel==0) continue;
		if(x.numposts>=currentuser.numposts) posts++;
		if(x.numlogins>=currentuser.numlogins) logins++;
		if(x.stay>=currentuser.stay) stays++;
		if(x.firstlogin<=currentuser.firstlogin) lifes++;
		total++;
	}
	close(fd);
	printf("<center>%s -- ��������ͳ�� [ʹ����: %s]<hr color=green>\n",
		BBSNAME, currentuser.userid);
	printf("<table class=body width=320><tr><th class=body>��Ŀ<th class=body>��ֵ<th class=body>ȫվ����<th class=body>��Ա���\n");
	printf("<tr><td class=body1>��վ����<td class=body1>%d��<td class=body1>%d<td class=body1>TOP %5.2f%%", 
		(time(0)-currentuser.firstlogin)/86400, lifes, (lifes*100.)/total);
	printf("<tr><td class=body2>��վ����<td class=body2>%d��<td class=body2>%d<td class=body2>TOP %5.2f%%",
		currentuser.numlogins, logins, logins*100./total);
	printf("<tr><td class=body1>��������<td class=body1>%d��<td class=body1>%d<td class=body1>TOP %5.2f%%",
		currentuser.numposts, posts, posts*100./total);
	printf("<tr><td class=body2>����ʱ��<td class=body2>%d��<td class=body2>%d<td class=body2>TOP %5.2f%%",
		currentuser.stay/60, stays, stays*100./total);
	printf("</table><br>���û���: %d", total);
	http_quit();
	return 0;
}

int search_online() {
	int i, total=0, total2=0; 
	struct user_info *x;
	struct user_info user[MAXACTIVE];
	char search;
	init_all();
	modify_user_mode(LAUSERS);
	printf("<center>\n");
	printf("%s -- �����û���ѯ [����������: %d��]<hr>\n", BBSNAME, count_online());
	for(i=0; i<MAXACTIVE; i++) {
		x=&(shm_utmp->uinfo[i]);
		if(x->active==0) continue;
		if(x->invisible && !HAS_PERM(PERM_SEECLOAK)) continue;
		memcpy(&user[total], x, sizeof(struct user_info));
		total++;
	}
	search=mytoupper(getparm("search")[0]);
	if(search!='*' && (search<'A' || search>'Z')) http_fatal("����Ĳ���");
	if(search=='*') {
		printf("��������ʹ����<br>\n");
	} else {
		printf("��ĸ'%c'��ͷ������ʹ����.<br>\n", search);
	}

	printf("<table class=body border=1 width=610>\n");
	printf("<tr><th class=body>���<th class=body>��<th class=body>ʹ���ߴ���<th class=body>ʹ�����ǳ�<th class=body>����<th class=body>��̬<th class=body>����\n");
	qsort(user, total, sizeof(struct user_info), cmpuser);
	for(i=0; i<total; i++) {
		int dt=(time(0)-user[i].idle_time)/60;
		if(mytoupper(user[i].userid[0])!=search && search!='*') continue;
		printf("<tr><td class=body%d>%d", i%2+1, i+1);
		printf("<td class=body%d>%s", i%2+1, isfriend(user[i].userid) ? "��" : "  ");
		printf("%s", user[i].invisible ? "<font color=green>C</font>" : " ");
		printf("<td class=body%d><a href=bbsqry?userid=%s>%s</a>", i%2+1, user[i].userid, user[i].userid);
		printf("<td class=body%d><a href=bbsqry?userid=%s>%24.24s </a>", i%2+1, user[i].userid, nohtml(user[i].username));
        if (!SHOW_IP((&(user[i]))))
                printf("<td class=body%d>ĳĳIP", i%2+1);
	else printf("<td class=body%d>%20.20s ", i%2+1, user[i].from);
		printf("<td class=body%d>%s", i%2+1, user[i].invisible ? "������..." : ModeType(user[i].mode));
		if(dt==0) {
			printf("<td class=body%d> \n", i%2+1);
		} else {
			printf("<td class=body%d>%d\n", i%2+1, dt);
		}
		total2++;
	}
	printf("</table>\n");
	printf("��������: %d��", total2);
	printf("<hr>");
	printf("<table class=foot>\n");
        if(search!='*') printf("<th class=foot><a href='bbsufind?search=*'>ȫ��</a> ");
        for(i='A'; i<='Z'; i++) {
		if(i==search) {
			printf("<th class=foot>%c", i);
		} else {
                	printf("<th class=foot><a href=bbsufind?search=%c>%c</a>", i, i);
		}
	}
	printf("</table>\n");
        printf("<br>\n");
        printf("<table class=foot>\n");
	printf("<th class=foot><a href='javascript:history.go(-1)'>����</a><th class=foot><a href=bbsusr>һ��ģʽ</a> ");
        printf("</table>\n");
	printf("</center>\n");
	http_quit();
	return 0;
}

int count_signature() {
	char fname[80];
	int count = 0;
	FILE *fp;
	char buf[256];

	setuserfile(fname, "signatures");
	fp = fopen(fname, "r");
	if (fp == NULL) return 0;
	while (1) {
		if (fgets(buf, sizeof(buf), fp) == 0) break;
		count++;
	}
	return count / 6;
}

/*--------babydragon: new register system-------------*/
/* <------- Added by betterman 06/07/27 -------> */
int 
valid_day(int year, int month, int day){	
	if (day < 1)
		return 0;	
	if (month == 2) {	
		if ((year % 4 != 0) || ((year % 400 != 0) && (year % 100 == 0))) {			
			if (day >= 29)				
				return 0;	
		} else {	
			if (day > 29)				
				return 0;		
		}
	}
	if ((month < 8 && month % 2 == 1) || (month >= 8 && month % 2 == 0)) {	
		if (day > 31)	
			return 0;
	} 
	else {	
		if (day >= 31)			
			return 0;
	}	
	return 1;
}


int
auth_fillform(struct userec *u, int unum)
{
	char buf[STRLEN];
	char secu[STRLEN];
	struct new_reg_rec regrec;
	struct userec newinfo;
	struct user_info *uinfo;
	FILE *authfile;
	int fail_count = 0;
	int fore_user = (u->userlevel != PERM_BASIC);	

	if(strcmp(u->userid,"SYSOP") == 0)
		return 1;
	search_ulist(&uinfo, u->userid);

#ifdef PASSAFTERTHREEDAYS
	if (u->lastlogin - u->firstlogin < 3 * 86400) {
		http_fatal("���״ε��뱾վδ������(72��Сʱ)...\"�����Ĵ���Ϥһ�£����������Ժ��ټ����ʺš�");
		return 0;
	}
#endif
	memset(&regrec, 0, sizeof(regrec));
	regrec.regtime = time(NULL);
	strlcpy(buf, getparm("pass"), sizeof(buf));
	if (!buf[0])
		http_fatal("����������");
	else if(!checkpasswd2(buf, &currentuser))
		http_fatal("�������");
	strlcpy(buf, getparm("grad"), sizeof(buf));
	if(buf[0] && (atoi(buf) <= 1920 || atoi(buf) >= 2020))
		http_fatal("����ı�ҵ���");
	if (!buf[0]) 
		http_fatal("����ѡ���ҵ���");
	regrec.graduate = atoi(buf);

	strlcpy(buf, getparm("real"), sizeof(buf));
	if(!buf[0])
		http_fatal("����������");
	strlcpy(regrec.rname, buf, sizeof(regrec.rname));
	//if (!regrec.rname[0]) 
	//	strlcpy(regrec.rname, u->realname, sizeof(u->realname));

	strlcpy(buf, getparm("year"), sizeof(buf));
	if(!buf[0])
		http_fatal("�������������");
	if(atoi(buf) <= 1920 || atoi(buf) >= 1998)
		http_fatal("����ĳ������");
	regrec.birthyear = atoi(buf) - 1900;

	strlcpy(buf, getparm("month"), sizeof(buf));
	if(!buf[0])
		http_fatal("�������������");
	if(atoi(buf) < 1 || atoi(buf) > 12)
		http_fatal("����ĳ����·�");
	regrec.birthmonth = atoi(buf);

	strlcpy(buf, getparm("day"), sizeof(buf));
	if(!buf[0])
		http_fatal("�������������");
	if(atoi(buf) < 1 || atoi(buf) > 31)
		http_fatal("����ĳ�����");
	regrec.birthday = atoi(buf);
	
	if(!valid_day(regrec.birthyear, regrec.birthmonth, regrec.birthday))
		http_fatal("����ĳ�������");
	
	strlcpy(buf, getparm("acc"), sizeof(buf));
	if (!buf[0])
		http_fatal("������ѧ��");
	strlcpy(regrec.account, buf, sizeof(regrec.account));
	
	strlcpy(buf, getparm("addr"), sizeof(buf));
	strlcpy(regrec.addr, buf, sizeof(regrec.addr));
	if(!regrec.addr[0] || strlen(regrec.addr) < 4 )
		http_fatal("����ĵ�ַ");
	
	strlcpy(buf, getparm("phone"), sizeof(buf));
        strlcpy(regrec.phone, buf, sizeof(regrec.phone));
	if(!regrec.phone[0] || strlen(regrec.phone) < 8)
		http_fatal("����ĵ绰����");
	
	strlcpy(buf, getparm("dept"), sizeof(buf));
	strlcpy(regrec.dept, buf, sizeof(regrec.dept));
	if (!regrec.dept[0])
		http_fatal("������רҵ");
	if(regrec.dept[strlen(regrec.dept) - 1] == 10
	|| regrec.dept[strlen(regrec.dept) - 1] == 13)
		regrec.dept[strlen(regrec.dept) - 1] = '\0';//babydragon: ȥ��CR��LF���з�
	if(check_auth_info(&regrec) > 0){

#define MULTIAUTH 3
		if(multi_auth_check(regrec.auth) >= MULTIAUTH){
			http_fatal(" ����: ��ǰ�����Ѿ��������! ��������!");
			return 0;
		}				

		memcpy(&newinfo, u, sizeof (*u));
		strcpy(newinfo.realname, regrec.rname);
		newinfo.birthyear = regrec.birthyear;		
		newinfo.birthmonth = regrec.birthmonth;
		newinfo.birthday = regrec.birthday;
        	newinfo.lastjustify = time(NULL);
		strcpy(newinfo.address, regrec.addr);
		memcpy(newinfo.reginfo, regrec.auth, MD5_PASSLEN);
		newinfo.reginfo[MD5_PASSLEN] = 0;
		memcpy(u, &newinfo, sizeof(currentuser));
        	newinfo.userlevel |= ( PERM_WELCOME | PERM_DEFAULT ); // ����ͨ�����û���ȡ����Ȩ�޺�perm_welcomeȨ

        	//if (deny_me_fullsite()) newinfo.userlevel &= ~PERM_POST;
		
        	if (substitute_record(PASSFILE, &newinfo, sizeof (newinfo), unum) == -1) {
       			http_fatal("ϵͳ��������ϵϵͳά��Ա\n");
			return 0;
		}else{
			if(fore_user){ /* ���û� */
				post_mail(currentuser.userid, "��ϲ����������ڸ��س���Argo��", 
							"etc/Activa_fore_users", "SYSOP", NULL, NULL, -1, 0);
			}else{ /* ���û� */
				post_mail(currentuser.userid, "��ϲ�������Ѿ����ע�ᡣ", 
							"etc/s_fill", "SYSOP", NULL, NULL, -1, 0);
				post_mail(currentuser.userid, "��ӭ���뱾վ���С�", 
							"etc/smail", "SYSOP", NULL, NULL, -1, 0);
			}
			snprintf(secu, sizeof(secu), "���� %s ���ʺ�", newinfo.userid);
			//todo : �ı�����ʽ
			securityreport2(secu, 1, NULL);
        	}
		
		setuserfile(buf, ".regpass"); // ɾ����֤�� 
		unlink(buf);
		setuserfile(buf, "auth");
		if (dashf(buf)) {
			setuserfile(genbuf, "auth.old");
			rename(buf, genbuf);
		}
		if ((authfile = fopen(buf, "w")) != NULL) {
			fprintf(authfile, "unum: %d, %s", unum,
				ctime(&(regrec.regtime)));
			fprintf(authfile, "userid: %s\n", u->userid);
			fprintf(authfile, "realname: %s\n", regrec.rname);
			fprintf(authfile, "dept: %s\n", regrec.dept);
			fprintf(authfile, "addr: %s\n", regrec.addr);
			fprintf(authfile, "phone: %s\n", regrec.phone);
			fprintf(authfile, "birthday: %d\n",regrec.birthyear + 1900);			
			fprintf(authfile, "birthday: %d\n",regrec.birthmonth);
			fprintf(authfile, "birthday: %d\n",regrec.birthday);
			fprintf(authfile, "graduate: %d\n",regrec.graduate);
			fprintf(authfile, "auth: %s\n",regrec.auth + 1);
			time_t now = time(NULL);
			fprintf(authfile, "Date: %s", ctime(&now));
			fprintf(authfile, "Approved: %s", u->userid);
			fclose(authfile);
		}
		//http_fatal("������!! �����ʺ���˳������.\n");
		hs_init(1);
		hs_setfile("bbsauthok.ptn");
		hs_end();
		return 1;
	}else{
		fail_count = atoi(getparm("fail_count")) + 1;
		if(fail_count > 3) {
			http_fatal("�����������ύ�ֹ���֤, ��Ⱥ�");
		}
		else if(fail_count == 3) {
			regrec.regtime = time(NULL);
			regrec.usernum = uinfo->uid;
			strcpy(regrec.userid, currentuser.userid);
			regrec.Sname = count_same_reg(regrec.rname, '1', 0);
			regrec.Slog = count_same_reg(currentuser.lasthost, '3', 0);
			regrec.Sip = count_same_reg(currentuser.ident, '2', 0);
			regrec.mark = ' ';
			if(append_record("new_register.rec", (void *)&regrec, sizeof (regrec)))
				http_fatal("��Ӽ�¼����");
			printf("<script>document.cookie='fail_count=%d'</script>", fail_count);
			http_fatal("�����������ύ�ֹ���֤!");
		} else {	
			printf("<script>document.cookie='fail_count=%d'</script>", fail_count);
			http_fatal("���������޷�ͨ����֤, ��������д<br/>"
				"<font color='ff0000'>�����֤ʧ��3�ν���ת���ֹ���֤</font><br>"
				"������%d�λ���", 3 - fail_count);
		}
		return 0;
	}
	return 0;
}

int check_auth_info(struct new_reg_rec *regrec)
{
	FILE *fp;
	char name[STRLEN], dept[STRLEN], birth[STRLEN], account[STRLEN], birthday[11];
	char buf[256], path[256];
	char *ptr, *ptr2;
	int passover;

#define 	setauthfile(buf, graduate)	sprintf(buf, "auth/%d/%d", graduate,graduate)
	setauthfile(path, regrec->graduate);

	/* add by betterman 06/10/10*/
	if(regrec->graduate == 2003)
		return -1;

	/* add by rovingcloud 09/05/21*/
	if(strstr(regrec->account, "033521") == regrec->account) 
		return -1;
	
	fp = fopen(path,"r");
	if(fp == NULL){
		http_fatal("û�иñ�ҵ��ݵ�����, ����ʧ��");
		return -1;
	}

	sprintf(birthday, "%.4d-%.2d-%.2d", regrec->birthyear + 1900, regrec->birthmonth, regrec->birthday);
	while(fgets(buf,256,fp) != NULL)
	{
		passover = 0;
		ptr = ptr2 = buf;
		if(*ptr == '\0' || *ptr == '\n' || *ptr == '\r' || *ptr == '#')
			continue;

		if((ptr = strchr(ptr,';')) == NULL)	
			continue;
		if (ptr - ptr2 - 1 > STRLEN)
			continue;
		strlcpy(name, ptr2, ptr - ptr2 +1);
		if(strcmp(name, regrec->rname) != 0)
			continue;
		if((ptr2 = strchr(ptr+1,';')) == NULL)	
			continue;		
		if (ptr2 - ptr - 1 > STRLEN)
			continue;
		strlcpy(dept, ptr+1, ptr2 - ptr );
		if(strlen(dept) == 0 ){//ȱרҵ�ֶ�
			passover++;
		}else if(strcmp(dept, regrec->dept) != 0)
			continue;
		if((ptr = strchr(ptr2+1,';')) == NULL)	
			continue;		
		if (ptr - ptr2 - 1 > STRLEN)
			continue;
		strlcpy(account, ptr2+1, ptr - ptr2 );
		if(strlen(account) == 0 ){//ȱѧ���ֶ�
			passover++;
		}else if(strcmp(account,regrec->account) != 0 &&
		            (strncmp(account,"0",1) !=0 || strcmp(account+1,regrec->account) != 0 ) ) // ģ����λ��0 
			continue;

		if((ptr2 = strchr(ptr+1,';')) == NULL)	
			continue;		
		if (ptr2 - ptr - 1 > STRLEN)
			continue;
		strlcpy(birth, ptr+1, ptr2 - ptr );
		if (strlen(birth) == 7){ //�����ֶ�����: 1985-11
			if(strncmp(birth,birthday,7) != 0 ) 
				continue;
		}else if(strlen(birth) == 10){  //�����ֶ�����: 1985-11-16
			if(strcmp(birth,birthday) != 0 && 
			   (strncmp(birth, birthday, 7)!=0 || strncmp(birth+8,"01",2)!=0 ) &&  // ����¼������������01�Ŷ��û������������ͨ�� 
			   (strncmp(birth, birthday, 4)!=0 || strncmp(birth+5,"01-01",5)!=0  ) ) // ����¼������������01-01���û����������ͨ�� 
				continue;
		}
		else // ȱ�����ֶλ����ֶγ��Ȳ����Ϲ淶 
			passover++;
		if(passover >= 2) // ȱ���������������ֶ���ͨ�� 
			continue;
		
		igenpass(buf, regrec->rname, regrec->auth);
		return 1;

	}
	fclose(fp);
	return 0;
}

int
countauths(void *uentp_ptr, int unused)
{
	static int totalusers;
	struct userec *uentp = (struct userec *)uentp_ptr;

	if (uentp == NULL) {
		int c = totalusers;

		totalusers = 0;
		return c;
	}
	if (uentp->userlevel & (PERM_WELCOME | PERM_BASIC) && 
                    memcmp(uentp->reginfo,genbuf, MD5_PASSLEN) == 0) // alarm: strncmp , but not strcmp
		totalusers++;
	return 0;
}

int multi_auth_check(char auth[MD5_PASSLEN])
{
	strncpy(genbuf, auth,MD5_PASSLEN);
	countmails(NULL, 0);
	if (apply_record(PASSFILE, countauths, sizeof (struct userec)) == -1) {
		return 0;
	}
	return countmails(NULL, 0);	
}
