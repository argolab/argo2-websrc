#include "webbs.h"

int 
cmpregrec (username, rec)
void *username;
void *rec;
{
        if (!strcmp((char *)username, ((struct new_reg_rec *)rec)->userid))
                return 1;
        else
                return 0;
}

int 
has_fill_form () {
        FILE *fp;
        struct new_reg_rec regrec;
        fp=fopen("new_register.rec", "r");
        if(fp==0) return 0;
        if (search_record("new_register.rec", &regrec, sizeof(regrec),
                cmpregrec, currentuser.userid))
                return 1;
        fclose(fp);
        return 0;  
}

int 
count_same_reg (username, type, myecho)
char *username;
char type;
int myecho;
{
        FILE *fp;
        int count = 0, x, y;
        struct userec rec;  
        char buf[STRLEN];   
        
        fp = fopen(PASSFILE, "r");
        if (!fp)  return 0;
        count = 0;
        x = 0;
        y = 4;
        while (fread(&rec, sizeof (rec), 1, fp) > 0) {
                switch (type) {
                case '1':
                        strlcpy(buf, rec.realname, sizeof(buf));
                        break;
                case '2':   
                        strlcpy(buf, rec.ident, sizeof(buf));
                        break;
                case '3':
                        strlcpy(buf, rec.lasthost, sizeof(buf));
                        break;
                }
                if (!strcmp(buf, username)) {
                        count++;
                }
        }
        fclose(fp);
        return count;
}

int check_register_ok(void)
{
   FILE   *fn;
   char   fname[STRLEN],genbuf[500];

   sprintf(fname, "%s/%s", BBSHOME, PASSFILE);
   setuserfile(fname, "register");
   if ((fn = fopen(fname, "r")) != NULL) {
      fgets(genbuf, STRLEN, fn);
      fclose(fn);
      strtok(genbuf, "\n");
      if (   ((strchr(genbuf, '@') != NULL) || strstr(genbuf, "usernum"))) {
         return 1;
      }
   }
   return 0;
}

int show_register_form() {
	/*
	if (validate_ip_range(fromhost) == 0) {
		http_fatal("非合法ip段用户不能注册帐号");
	}
	*//* Modified by betterman 06.09.04 */
	if (dashf("etc/NOREGISTER")) { //babydragon: 如果暂停注册的文件存在
		http_fatal("本站暂停注册");
	}
	init_all();
	modify_user_mode(NEW);
	hs_init(1);
	hs_setfile("bbsreg.ptn");
	hs_end();
	http_quit();
	return 0;
}

#define PAGER_FLAG      0x01    /* true if pager was OFF last session */
#define CURSOR_FLAG     0x80    /* true if the cursor mode open */ 
#define FLUSH       ".PASSFLUSH"        /* Stores date for user cache flushing */

int id_with_num(char *userid)
{
        char *s;
                        
        for (s = userid; *s != '\0'; s++)
                if (*s < 1 || !isalpha(*s))
                        return 1;
        return 0; 
}  


int
bad_user_id(char *userid)
{
	FILE *fp;
	char *ptr;
	char pattern[1024];

	if ((fp = fopen("etc/bad_id", "r")) != NULL) {
#ifdef FNM_CASEFOLD
		while (fgets(pattern, sizeof(pattern), fp)) {
			if (pattern[0] == '#')
				continue;

			if ((ptr = strrchr(pattern, '\n')) != NULL)
				*ptr = '\0';

			if (fnmatch(pattern, userid, FNM_CASEFOLD) == 0) {
				fclose(fp);
				return YEA;
			}	
		}
#else
		char lpattern[1024], luserid[1024];

		strtolower(luserid, userid);
		while (fgets(pattern, sizeof(pattern), fp)) {
			if (pattern[0] == '#')
				continue;

			if ((ptr = strrchr(pattern, '\n')) != NULL)
				*ptr = '\0';

			strtolower(lpattern, pattern);
			if (fnmatch(lpattern, luserid, 0) == 0) {
				fclose(fp);
				return YEA;
			}	
		}		
#endif
	}
	fclose(fp);
	return NA;
}


int badymd(int y, int m, int d) {
	int max[]={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if((y%4==0 && y%100!=0) || y%400==0) max[2]=29;
	if(y<10 || y>100 || m<1 || m>12) return 1;
	if(d<0 || d>max[m]) return 1;
	return 0;
} 

int m_userid_validcate() {
	char userid[IDLEN + 2];
	char *message = "";
	int valid = 0;
	int i = 0;
 
	init_all();

	strlcpy(userid, getparm("userid"), IDLEN + 2);

	// Starts checking 
	printf("<?xml version=\"1.0\" encoding=\"gb2312\"?>\n"
	       "<pair>\n");
	
	for(i = 0; userid[i]; i++) {
      		if(!strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", mytoupper(userid[i]))) {
			message = "帐号只能由英文字母组成, 请重新选择。";
			goto done;
		}
	}
   	if(strlen(userid)<2) {
		message = "帐号长度太短(2-12字符), 请重新选择。";
		goto done;
	}
 	if(bad_user_id(userid)) {
		message = "不雅帐号或禁止注册的id, 请重新选择。";
		goto done;
	}
   	if(getuser(userid)) {
		message = "此帐号已经有人使用, 请重新选择。";
		goto done;
	}
   	
	valid = 1;
done:
	printf("<valid>%d</valid>\n", valid);
	printf("<message> %s </message>\n", message);
	printf("<userid>%s</userid>\n", userid);
	printf("</pair>");
	http_quit();
	return 0;
}

int do_register() {
	struct userec x;
        //struct new_reg_rec regrec;
	int i, xz;
	char buf[80], filename[80], pass1[80], pass2[80], dept[80], phone[80], salt[80], assoc[80];
   	init_all();
 	if (dashf("etc/NOREGISTER")) { //babydragon: 如果暂停注册的文件存在
		http_fatal("本站暂停注册");
	}
	memset(&x, 0, sizeof(x));
	xz=atoi(getparm("xz"));
  	strlcpy(x.userid, getparm("userid"), IDLEN + 2);
   	strlcpy(pass1, getparm("pass1"), MD5_PASSLEN);
   	strlcpy(pass2, getparm("pass2"), MD5_PASSLEN);
   	strlcpy(x.username, getparm("username"), NICKNAMELEN + 1);
   	strlcpy(x.realname, getparm("realname"), NAMELEN + 1);
   	strsncpy(dept, getparm("dept"), 32);
   	strsncpy(x.address, getparm("address"), 32);
   	strsncpy(x.email, getparm("email"), 32);
   	strsncpy(phone, getparm("phone"), 32);
	strsncpy(assoc, getparm("assoc"), 32);
//	strsncpy(words, getparm("words"), 1000);
	x.gender='M';
	if(atoi(getparm("gender"))) x.gender='F';
	x.birthyear=atoi(getparm("year"))-1900;
	x.birthmonth=atoi(getparm("month"));
	x.birthday=atoi(getparm("day"));
   	for(i=0; x.userid[i]; i++)
      		if(!strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", mytoupper(x.userid[i]))) http_fatal("帐号只能由英文字母组成");
   	if(strlen(x.userid)<2) http_fatal("帐号长度太短(2-12字符)");
   	if(strlen(pass1)<4) http_fatal("密码太短(至少4字符)");
   	if(strcmp(pass1, pass2)) http_fatal("两次输入的密码不一致, 请确认密码");
	if (strcmp(pass1, x.userid) == 0) 
		http_fatal("用户名和密码相同，请重新指定一个新密码");
   	if(strlen(x.username)<2) http_fatal("请输入昵称(昵称长度至少2个字符)");
   	if(strlen(x.realname)<4) http_fatal("请输入真实姓名(请用中文, 至少2个字)");
   	if(strlen(dept)<6) http_fatal("工作单位的名称长度至少要6个字符(或3个汉字)");
   	if(strlen(x.address)<6) http_fatal("通讯地址长度至少要6个字符(或3个汉字)");
	if (strlen(x.email) < 3) http_fatal("无效的email地址");
   	if(id_with_num(x.userid)||badstr(x.passwd)||badstr(x.username)||badstr(x.realname)) 
		http_fatal("您的注册单中含有非法字符");
	if(badstr(dept)||badstr(x.address)||badstr(x.email)||badstr(phone)||badstr(assoc)) http_fatal("您的注册单中含有非法字符");
   	if(badymd(x.birthyear, x.birthmonth, x.birthday)) http_fatal("请输入您的出生年月");
	if(bad_email(x.email)) http_fatal("电子邮件地址无效");
 	if(bad_user_id(x.userid)) http_fatal("不雅帐号或禁止注册的id, 请重新选择");
   	if(getuser(x.userid)) http_fatal("此帐号已经有人使用,请重新选择。");
//	sprintf(salt, "%c%c", 65+rand()%26, 65+rand()%26);
	sprintf(salt, "%s", x.userid);
   	strsncpy(x.passwd, crypt_des(pass1, salt), 14);
   	strlcpy(x.termtype, "vt100", sizeof(x.termtype));
   	strlcpy(x.lasthost, fromhost, sizeof(x.lasthost));
	strlcpy(x.ident, fromhost, sizeof(x.ident));
   	x.userlevel=PERM_BASIC;
   	x.firstlogin=time(0);
	x.lastlogin=time(0);
   	x.userdefine=-1;
   	x.flags[0]=CURSOR_FLAG | PAGER_FLAG;
//	if(xz==1) currentuser.userdefine ^= DEF_COLOREDSEX;
//	if(xz==2) currentuser.userdefine ^= DEF_S_HOROSCOPE;
	adduser(&x);

/*
	regrec.usernum=getusernum(x.userid)+1;
	regrec.regtime=time(0);
	strlcpy(regrec.userid, x.userid, sizeof(regrec.userid));  
	strlcpy(regrec.rname, x.realname, sizeof(regrec.rname));
	strlcpy(regrec.addr, x.address, sizeof(regrec.addr));
	strsncpy(regrec.phone, phone, STRLEN);
	strsncpy(regrec.dept, dept, STRLEN);
	strsncpy(regrec.assoc, assoc, STRLEN);
        regrec.Sname = count_same_reg(x.realname, '1', NA);
        regrec.Slog = count_same_reg(x.lasthost, '3', NA);
        regrec.Sip = count_same_reg(x.ident, '2', NA);
	regrec.Sip=-1;
	regrec.Sname=-1;
	regrec.Slog=-1;
	regrec.mark=' ';
        append_record("new_register.rec", &regrec, sizeof (regrec));
*/
	
/*
   	fp=fopen("new_register", "a");
	if(fp) {
      		fprintf(fp, "usernum: %d, %s\n", 	getusernum(x.userid)+1, Ctime(time(0)));
      		fprintf(fp, "userid: %s\n",    	x.userid);
      		fprintf(fp, "realname: %s\n",  	x.realname);
      		fprintf(fp, "dept: %s\n",    	dept);
      		fprintf(fp, "addr: %s\n",      	x.address);
      		fprintf(fp, "phone: %s\n",     	phone );
//      		fprintf(fp, "assoc:\n");
      		fprintf(fp, "----\n" );
      		fclose(fp);
   	}
*/
//   	file_append("wwwlog/trace.post", "G");
	sethomepath(filename, x.userid);
//   	sprintf(filename, "home/%c/%s", toupper((int)x.userid[0]), x.userid);
   	f_mkdir(filename, 0755);
	
	/*printf("<center><table><td><td><pre>\n");
	printf("亲爱的新使用者，您好！\n\n");
        printf("欢迎光临 本站, 您的新帐号已经成功被登记了。\n");
        printf("您目前拥有本站基本的权限, 包括阅读文章、环顾四方、接收私人\n");
	printf("信件、接收他人的消息、进入聊天室等等。当您通过本站的身份确\n");
	printf("认手续之后，您还会获得更多的权限。目前您的注册单已经被提交\n");
	printf("等待审阅。一般情况24小时以内就会有答复，请耐心等待。同时请\n");
	printf("留意您的站内信箱。\n");
	printf("如果您有任何疑问，可以去BBS_Help(BBS使用求助)版发文求助。\n\n</pre></table>");
   	printf("<hr color=green><br>您的基本资料如下:<br>\n");
   	printf("<table border=1 width=400>");
   	printf("<tr><td>帐号位置: <td>%d\n", getusernum(x.userid));
   	printf("<tr><td>使用者代号: <td>%s (%s)\n", x.userid, x.username);
   	printf("<tr><td>姓  名: <td>%s<br>\n", x.realname);
	printf("<tr><td>昵  称: <td>%s<br>\n", x.username);
   	printf("<tr><td>上站位置: <td>%s<br>\n", x.lasthost);
   	printf("<tr><td>电子邮件: <td>%s<br></table><br>\n", x.email);
   	printf("<center>");
	printf("<form method=post action=bbslogin>");
	printf("<input type=hidden name=id value=%s><input type=hidden name=pw value=%s>",
			x.userid, pass1);
	printf("<input type=hidden name=type value=reg>");
	printf("<input type=submit value=进入本站>");
	printf("</form>");
	printf("<input type=button onclick='window.close()' value=关闭本窗口></center>\n");
*/
/*-------- no use in yat-sen bbs and will cause server error --------*/
//   	newcomer(&x, words);
   	sprintf(buf, "%s %-12s %d\n", Cdtime(time(0)), x.userid, getusernum(x.userid));
   	file_append("wwwlog/wwwreg.log", buf);
//	added by scs for newaccount
	sprintf(buf, "%s APPLY %-12s %d (wwwreg)\n", getdatestring(time(0)), x.userid, getusernum(x.userid));
	file_append("wwwlog/usies", buf);
        sprintf(buf, "touch by: %d\n", (int) time(0));
        file_append(FLUSH, buf);

	update_utmp();
	setcookie("utmpkey", "");
	setcookie("utmpnum", "");
	setcookie("utmpuserid", "");
	setcookie("my_t_lines", "");
	setcookie("my_link_mode", "");
	setcookie("my_def_mode", "");
	
	hs_init(5);
	hs_setfile("bbsregok.ptn");
	hs_assign("USER", x.userid);
	hs_assign("PASS", pass1);
	hs_end();
	http_quit();
	return 0;
}

int badstr(unsigned char *s) {
  	int i;
	for(i=0; s[i]; i++)
    		if(s[i]!=9 &&(s[i]<32 || s[i]==255)) return 1;
  	return 0;
}

int bad_email(unsigned char *s) {
	if (strstr(s, "@") == 0 || strcasestr(s, "bbs")) return 1;
	return badstr(s);
}

int newcomer(struct userec *x, char *words) {
  	FILE *fp;
  	char filename[80];
	sprintf(filename, "tmp/%d.tmp", getpid());
	fp=fopen(filename, "w");
	if (fp == NULL) return -1;
	fprintf(fp, "大家好, \n\n");
	fprintf(fp, "我是 %s(%s), 来自 %s\n", x->userid, x->username, fromhost);
	fprintf(fp, "今天初来此地报到, 请大家多多指教.\n\n");
	fprintf(fp, "自我介绍:\n\n");
	fprintf(fp, "%s", words);
	fclose(fp);
//	post_article("newcomers", "WWW新手上路", filename, x->userid, 
//x->username, fromhost, -1);
	unlink(filename);
	return 0;
}

int adduser(struct userec *x) {
	int i;
	FILE *fp;
	fp=fopen(".PASSWDS", "r+");
	if (fp == NULL) return -1;
	flock(fileno(fp), LOCK_EX);
	for(i=0; i<MAXUSERS; i++) {
		if(shm_ucache->userid[i][0]==0) {
			setuserid(i, x->userid);
			save_user_data(x);
			break;
		}
	}
	if (getuser(x->userid) == NULL) http_fatal("无法创建用户");
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	utime(".PASSFLUSH", NULL);
	return 0;
}

int fill_register_form() {
	int type;
	init_all();
	type=atoi(getparm("type"));
  	if(!loginok) http_fatal("您尚未登录, 请重新登录。");
	printf("%s -- 填写注册单<hr color=green>\n", BBSNAME);
	check_if_ok();
	if(type==1) {
		check_submit_form();
		http_term();
	}
  	printf("您好, %s, 注册单通过后即可获得注册用户的权限, 下面各项务必请认真填写<br><hr>\n", currentuser.userid);
  	printf("<form method=post action=bbsform?type=1>\n");
  	printf("真实姓名: <input name=realname type=text maxlength=8 size=8 value='%s'><br>\n", 
		nohtml(currentuser.realname));
  	printf("学校系级: <input name=dept type=text maxlength=32 size=32 value='%s'>(或工作单位)<br>\n", 
		nohtml(currentuser.reginfo));
  	printf("居住地址: <input name=address type=text maxlength=32 size=32 value='%s'><br>\n", 
		nohtml(currentuser.address));
	printf("<tr><td align=right>校友会:<td align=left><input name =assoc size=40><br>\n");
  	printf("联络电话: <input name=phone type=text maxlength=32 size=32>(没有可写'无')<br>\n");
  	printf("电子邮件: <input name=email type=text maxlength=32 size=32><br>\n");
	printf("<hr><br>\n");
  	printf("<input type=submit> <input type=reset>");
	http_quit();
	return 0;
}

int check_if_ok() {
  	if(user_perm(&currentuser, PERM_LOGINOK)) http_fatal("您已经通过本站的身份认证, 无需再次填写注册单.");
  	if(has_fill_form()) http_fatal("目前站长尚未处理您的注册单，请耐心等待.");
	return 0;
}

int check_submit_form() {
	struct new_reg_rec regrec;
	struct userec x;

	getdatestring(time(0));
        regrec.usernum=getusernum(currentuser.userid)+1;
        regrec.regtime=time(0);

	get_record(PASSFILE, &x, sizeof(x), regrec.usernum);
   	strsncpy(x.realname, getparm("realname"), sizeof(x.realname));
   	strsncpy(x.address, getparm("address"), sizeof(x.address));
   	strsncpy(x.email, getparm("email"), sizeof(x.email));
	if (strlen(x.email) < 3) http_fatal("无效的email地址");
	if (bad_email(x.email)) http_fatal("无效的email地址");
	substitute_record(PASSFILE, &x, sizeof(x), regrec.usernum);

        strsncpy(regrec.userid, currentuser.userid, IDLEN);
        strsncpy(regrec.rname, getparm("realname"), sizeof(regrec.rname));
        strsncpy(regrec.addr, getparm("address"), sizeof(regrec.addr));
        strsncpy(regrec.phone, getparm("phone"), sizeof(regrec.phone));
        strsncpy(regrec.dept, getparm("dept"), sizeof(regrec.dept));
        strsncpy(regrec.assoc, getparm("assoc"), sizeof(regrec.assoc));
        regrec.Sname = count_same_reg(currentuser.realname, '1', NA);
        regrec.Slog = count_same_reg(currentuser.lasthost, '3', NA);
        regrec.Sip = count_same_reg(currentuser.ident, '2', NA);    
 	regrec.mark=' ';
        append_record("new_register.rec", (void *)&regrec, sizeof (regrec));
  	printf("您的注册单已成功提交. 站长检验过后会给您发信, 请留意您的信箱.");
	return 0;
}

/*--------babydragon: new regester system-------------*/
/* <- Added by betterman ,modify by babydragon 2006/08 -> */

int show_activation_from(){
	int i, grad;
	char buf[256];
	char path[256];
	FILE *fp;
	
	grad = atoi(getparm("grad"));
	//if(atoi(buf) <= 1920 || atoi(buf) >= 2006)
	//	http_fatal("没有该毕业年份的资料");
	sprintf(path, "auth/%d/dept", grad);
	fp = fopen(path, "r");
	if(fp == NULL)
		http_fatal("没有该毕业年份的资料");
	printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
	printf("<html>\n");
	printf("<head>\n");
	printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	printf("<title>" BBSNAME "电子公告板系统（BBS）</title>\n");
	printf("<link href=\"templates/global.css\" rel=\"stylesheet\" type=\"text/css\" />\n");
	printf("<style type=\"text/css\">\n");
	printf("body{\n");
	printf("margin-right:16px;\n");
	printf("}\n");
	printf("</style>\n");
	printf("</head>\n");	
	printf("<body>\n");
	printf("<form name=\"form1\" method=\"post\" action=\"bbsauth?type=5\">\n");
	printf("<div id=\"head\">\n");
	printf("<div id=\"location\">\n");
	printf("<p><img src=\"images/location01.gif\" alt=\"\" align=\"absmiddle\"/><a href=\"bbsallsec\">" BBSNAME "电子公告板系统（BBS）</a></p>\n");
	printf("<p><img src=\"images/location03.gif\" alt=\"\" align=\"absmiddle\"/>激活帐号</p>\n");
	printf("</div>\n");
	printf("</div>\n");
	printf("<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"table\">\n");
	printf("<tr> \n");
	printf("<td colspan=\"2\" class=\"tb_head\"><img src=\"images/table_ul06.gif\" alt=\"\" width=\"37\" height=\"25\" align=\"absmiddle\" class=\"tb_ul\" style=\"float:left\" /> \n");
	printf("<div style=\"margin-top:8px\">1. 注册帐号 --> <font color=#0000ff>2. 激活帐号</font> --> 3. 畅游我站</div></td>\n");
	printf("<td align=\"right\" valign=\"top\" class=\"tb_r\"><img src=\"images/table_ur.gif\" alt=\"\"/></td>\n");
	printf("</tr>\n");
	printf("<tr>\n");
	printf("<td class=\"tb_l\"></td>\n");
	printf("<td>\n");
	printf("<ul class=\"search\">\n");
	printf("请正确填写以下资料(全部资料必须填写)<br/>"
		"由于资料不完整,可能出现验证不通过的情况."
		"如果验证失败<font color='ff0000'>3</font>次, 我们将会进行手工验证<br/>");
	printf("密码: <input type=\"password\" name=\"pass\"/><br/><br/>\n");
	printf("真实姓名: <input type=\"text\" name=\"real\"/><br/>\n");
	printf("<input type=\"hidden\" name=\"grad\" value=\"%d\">\n", grad);
	printf("</script>\n");
 	printf("出生日期: ");
	printf("<select name=\"year\">\n");
	printf("<option></option>");
	for(i = 1920; i <= 1998; i++){
		printf("<option value=\"%d\">%d</option>\n", i, i);
	}
	printf("</select>年");
	printf("<select name=\"month\">");
	printf("<option></option>");
	for(i = 1; i <= 12; i++){
		printf("<option value=\"%d\">%d</option>\n", i, i);
	}
	printf("</select>月\n");
	printf("<select name=\"day\">\n");
	printf("<option></option>");
	for(i = 1; i <= 31; i++){
		printf("<option value=\"%d\">%d</option>\n", i, i);
	}
	printf("</select>日<br/>\n");
	printf("学号: <input type=\"text\" name=\"acc\"/><br/>\n");
	printf("专业: <select name=\"dept\">\n");
	while(!feof(fp)){
		if(fgets(buf, sizeof(buf), fp)!=NULL && strlen(buf) > 1)
			printf("<option value=\"%s\">%s</option>\n", buf, buf);
	}
	printf("</select><br/>\n");
	printf("电子邮件: <input type=\"text\" name=\"email\"/><br/>\n");
	printf("电话: <input type=\"text\" name=\"phone\"/>(8位以上)<br/>\n");
	printf("住址: <input type=\"text\" name=\"addr\"/>(4位以上)<br/>\n");
	printf("<input type=\"submit\" value=\"提交\"/> <input type=\"reset\" value=\"重新填写\"/>\n");
	printf("</ul></td>\n");
	printf("<td class=\"tb_r\"> </td>\n");
	printf("</tr>\n");
	printf("<tr class=\"tb_bottom\"> \n");
	printf("<td width=\"10\"><img src=\"images/table_bl.gif\" alt=\"\"/></td>\n");
	printf("<td width=\"757\"></td>\n");
	printf("<td align=\"right\"><img src=\"images/table_br.gif\" alt=\"\"/></td>\n");
	printf(	"</tr>\n");
	printf("</table>\n");
	printf("<div id=\"footer\"></div>\n");
	printf("</form>\n");
	printf("</body>\n");
	printf("</html>\n");
	return 0;
}

// Added by betterman, 10/14/2007
int activation(char *userid, char *auth_code) {
	struct userec *urec;
	struct user_info *u;
	char code[1024], *ptr;
  	char buf[PATH_MAX + 1], secu[STRLEN];
  	FILE *fp;
	int fore_user = 0;

	if (userid == 0 || auth_code == 0) {
		http_fatal("非法的激活链接\n");
	}

	urec = getuser(userid);
	if(urec == 0) {
		http_fatal("非法的激活链接, 用户 [%s] 不存在.", userid);
	}
	fore_user = (urec->userlevel != PERM_BASIC);
	search_ulist(&u, urec->userid);	

	sethomefile(buf, urec->userid, ".regpass");
    	if (dashf(buf)) {
		if ((fp = fopen(buf, "r")) != NULL) {
			if (fgets(code, sizeof(code), fp) != NULL) {
          			if ((ptr = strrchr(code, '\n')) != NULL)
            				*ptr = '\0';
        		}
        		fclose(fp);
      		}
      	
		if(strcmp(auth_code, code) != 0){
       			http_fatal("非法的激活连接\n");
		}else{
#ifndef MAXMAIL
#define MAXMAIL 3
#endif

			if(multi_mail_check(urec->email) >= MAXMAIL){
				http_fatal("警告: 当前邮箱已经激活过多! 请勿再试!");
			}
        		//set_safe_record();
        		urec->lastjustify = time(NULL);
        		urec->userlevel |= (PERM_WELCOME | PERM_DEFAULT);
			// 激活通过的用户获取基本权限和perm_welcome权 
        		//if (deny_me_fullsite()) urec->userlevel &= ~PERM_POST;

			// 同时写进注册信息里面 
       			strlcpy(urec->reginfo, urec->email, sizeof(urec->reginfo));
			if (substitute_record(PASSFILE, urec, 
			    sizeof (struct userec), u->uid) == -1) {

				http_fatal("系统错误，请联系系统维护员\n");
       			} else {          				
				if(fore_user){ /* 旧用户 */
					post_mail(urec->userid, "恭喜，今后您可在各地畅游Argo。", 
					    "etc/Activa_fore_users", "SYSOP", NULL, NULL, -1, 0);
				}else{ /* 新用户 */
					post_mail(urec->userid, "恭喜您，您已经完成注册。", 
							"etc/s_fill", "SYSOP", NULL, NULL, -1, 0);
					post_mail(urec->userid, "欢迎加入本站行列。", 
							"etc/smail", "SYSOP", NULL, NULL, -1, 0);
				}
				snprintf(secu, sizeof(secu), "激活 %s 的帐号", urec->userid);
				securityreport2(secu, YEA, NULL);
	        		unlink(buf);
				http_fatal("激活成功, 请重新登录\n");
       			}
		}
	}else{
		http_fatal("非法的激活链接\n");
	}
}

int
m_activation()
{
  	struct userec *urec = &currentuser;
	struct user_info *u;
	char *type, auth_code[1024];
	char code[1024], *ptr;
  	FILE *fp;
  	char buf[PATH_MAX + 1], secu[STRLEN];
	int fore_user = (currentuser.userlevel != PERM_BASIC);
  	//if (guestuser)
    	//	return -1;
	
	
	init_all();

	type = getparm("type");
	strlcpy(auth_code, getparm("code"), sizeof(auth_code));

	if(!loginok){
//		if(*type == '1') {
//			activation(userid, auth_code);
//		}else{ 
			http_fatal("请先登陆");
//		}
	}
  	modify_user_mode(NEW);
	/*
	if (dashf("etc/Activation"))
		ansimore("etc/Activation", YEA);
	*/
  	if(HAS_PERM(PERM_WELCOME)){
      		http_fatal("你的帐号已经激活");
      		return 0;
    	}

	search_ulist(&u, urec->userid);

	switch (*type) {
	case '1':
		//输入验证码
		// Add something to check the email
		sethomefile(buf, currentuser.userid, ".regpass");
    		if (dashf(buf)) {
			if(*auth_code == 0){
				hs_init(1);
				hs_setfile("bbschkcode.ptn");
				hs_end();
				break;				
			}	
      			if ((fp = fopen(buf, "r")) != NULL) {
        			if (fgets(code, sizeof(code), fp) != NULL) {
          				if ((ptr = strrchr(code, '\n')) != NULL)
            					*ptr = '\0';
        			}
        			fclose(fp);
      			}
      			if(strcmp(auth_code, code) != 0){
        			http_fatal("输入验证码错误");
      			}else{
#ifndef MAXMAIL
#define MAXMAIL 3
#endif
				if(multi_mail_check(currentuser.email) >= MAXMAIL){
					http_fatal("警告: 当前邮箱已经激活过多! 请勿再试!");
					break;
				}
        			//set_safe_record();
        			urec->lastjustify = time(NULL);
        			urec->userlevel |= (PERM_WELCOME | PERM_DEFAULT);
				// 激活通过的用户获取基本权限和perm_welcome权 
        			//if (deny_me_fullsite()) urec->userlevel &= ~PERM_POST;

				// 同时写进注册信息里面 
       				strlcpy(urec->reginfo, urec->email, sizeof(urec->reginfo));
				if (substitute_record(PASSFILE, urec, sizeof (struct userec), u->uid) == -1) {
					http_fatal("系统错误，请联系系统维护员\n");
        			} else {
					//mail_sysfile("etc/smail", currentuser.userid, "欢迎加入本站行列");
					//post_mail(currentuser.userid, "恭喜您！！ 您的帐号已顺利激活。", 
					//"/etc/smail", "SYSOP", NULL, NULL, -1, 0);
					if(fore_user){ /* 旧用户 */
						post_mail(currentuser.userid, "恭喜，今后您可在各地畅游Argo。", 
								"etc/Activa_fore_users", "SYSOP", NULL, NULL, -1, 0);
						//mail_sysfile("etc/Activa_fore_users", currentuser.userid, "恭喜，今后您可在各地畅游Argo");
					}else{ /* 新用户 */
						post_mail(currentuser.userid, "恭喜您，您已经完成注册。", 
							"etc/s_fill", "SYSOP", NULL, NULL, -1, 0);
						post_mail(currentuser.userid, "欢迎加入本站行列。", 
							"etc/smail", "SYSOP", NULL, NULL, -1, 0);
						//mail_sysfile("etc/s_fill", currentuser.userid, "恭喜您，您已经完成注册");
						//mail_sysfile("etc/smail", currentuser.userid, "欢迎加入本站行列");
					}
					snprintf(secu, sizeof(secu), "激活 %s 的帐号", urec->userid);
					securityreport2(secu, YEA, NULL);
	        			unlink(buf);
					//http_fatal("恭贺您!! 您的帐号已顺利激活.\n");
					hs_init(1);
					hs_setfile("bbsauthok.ptn");
					hs_end();
        			}

      			}
		}else{
      			http_fatal("请先获取验证码\n");
    		}
		break;
	case '2':
		//输入邮箱获取验证码
		ptr = getparm("user");
		if(*ptr == 0){
			hs_init(1);
			hs_setfile("bbsgetcode.ptn");
			hs_end();
			break;
		};
		sethomefile(buf, currentuser.userid, ".regpass");
		unlink(buf);
    		get_code(&currentuser);
		break;
	case '3':
		//选择毕业年份
//		if (validate_ip_range(fromhost))
//			http_fatal("错误: 目前该项功能仅限于已毕业的校友");
		hs_init(1);
		hs_setfile("bbsauth.ptn");
		hs_end();
		break;
	case '4':
		//填写个人资料,限校友使用
//		if (validate_ip_range(fromhost))
//			http_fatal("错误: 目前该项功能仅限于已毕业的校友");
		show_activation_from();
		break;
	case '5':
		//验证资料
//		if (validate_ip_range(fromhost))
//			http_fatal("错误: 目前该项功能仅限于已毕业的校友");
		auth_fillform(&currentuser, u->uid);
		break;
	default:
		//选择验证方式
		hs_init(1);
		hs_setfile("bbsdoreg.ptn");
		hs_assign("HOST", host_header);
		hs_end();
	}
	http_quit();
    	return 0;
}


int m_validate()
{
  	struct userec *urec = &currentuser;
	struct user_info *u;
	FILE *fp;
	char *ptr;
	char secu[STRLEN];
	char ticket[512];
	char command[1024];
	char output[1024];
	int fore_user = (currentuser.userlevel != PERM_BASIC);
	
	
	init_all();

	strlcpy(ticket, getparm("ticket"), sizeof(ticket));

	if(!loginok) {
		http_fatal("请先登陆");
	}
  	modify_user_mode(NEW);

  	if(HAS_PERM(PERM_WELCOME)){
      		http_fatal("你的帐号已经激活");
      		return 0;
    	}

	search_ulist(&u, urec->userid);
	
	sprintf(command, "%s/bin/validate_netid.py http://%s/bbsvalidate %s",
		BBSHOME, host_header, ticket);

	fp = popen(command , "r");
	if (fp == NULL) {
		http_fatal("系统错误，请联系管理员。");
		return;
	}
	/* 返回 no 或者 NetID */
	if (fgets(output, sizeof(output) - 1, fp) != NULL) {
		if ((ptr = strrchr(output, '\n')) != NULL)
			*ptr = '\0';
		
		if (strcmp(output, "no") == 0) {
			http_fatal("NetID 验证失败，请重新尝试。");
		}

		if(multi_netid_check(output) >= MAXMAIL) {
			http_fatal("警告: 当前 NetID 已经激活过多! 请勿再试!");
		}

		urec->lastjustify = time(NULL);
		urec->userlevel |= (PERM_WELCOME | PERM_DEFAULT);
		// 激活通过的用户获取基本权限和perm_welcome权 
		//if (deny_me_fullsite()) urec->userlevel &= ~PERM_POST;

		// 同时写进注册信息里面 
		strlcpy(urec->reginfo, output, sizeof(urec->reginfo));
		if (substitute_record(PASSFILE, urec, sizeof (struct userec), u->uid) == -1) {
			http_fatal("系统错误，请联系系统维护员\n");
		} else {
			if (fore_user){ /* 旧用户 */
				post_mail(currentuser.userid, "恭喜，今后您可在各地畅游Argo。", 
					  "etc/Activa_fore_users", "SYSOP", NULL, NULL, -1, 0);
			}else{ /* 新用户 */
				post_mail(currentuser.userid, "恭喜您，您已经完成注册。", 
					  "etc/s_fill", "SYSOP", NULL, NULL, -1, 0);
				post_mail(currentuser.userid, "欢迎加入本站行列。", 
					  "etc/smail", "SYSOP", NULL, NULL, -1, 0);
			}
			snprintf(secu, sizeof(secu), "激活 %s 的帐号", urec->userid);
			securityreport2(secu, YEA, NULL);
			//http_fatal("恭贺您!! 您的帐号已顺利激活.\n");
			hs_init(1);
			hs_setfile("bbsauthok.ptn");
			hs_end();
		}
		return;
	}
	
	http_fatal("系统错误，请联系管理员。");
	return;
}

#ifdef CODE_VALID
char *
genrandpwd(int seed)
{
	static char panel[] =
		"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *result;
	int i, rnd;


	result = (char *)malloc(RNDPASSLEN + 1);
	srand((unsigned)(time(NULL) * seed));
	memset(result, 0, RNDPASSLEN + 1);
	for (i = 0; i < RNDPASSLEN; i++) {
		rnd = rand() % sizeof(panel);
		if (panel[rnd] == '\0') {
			i--;
			continue;
		}
		result[i] = panel[rnd];
	}
	sethomefile(genbuf, currentuser.userid, ".regpass");
	unlink(genbuf);
	file_append(genbuf, result);
	return result;
}
#endif

void
send_regmail(struct userec *trec)
{
	FILE *fout, *fp;
	time_t code = time(NULL);
	pid_t pid = getpid();
#ifdef CODE_VALID
	char buf[RNDPASSLEN + 1];
#endif
	char temp[STRLEN];
	if ((fp = fopen("etc/bad_email", "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			if (strstr(temp, trec->email) != NULL) {
				fclose(fp);
				http_fatal("警告: 非法邮箱! 请勿再试!");
			}
		}
		fclose(fp);
	}

/*
	sethomefile(genbuf, trec->userid, "mailcheck");
	if ((dp = fopen(genbuf, "w")) == NULL)
		http_fatal("can't creat file");
	fprintf(dp, "%9.9d:%d\n", (int)code, (int)pid);
	fclose(dp);
*//* Canceled by betterman 06.07 */

	snprintf(genbuf, sizeof(genbuf), "%s -f new.bbs@%s %s", SENDMAIL, BBSHOST, trec->email);
	if ((fout = popen(genbuf, "w")) == NULL) {
		http_fatal("sendmail error");
	}

	fprintf(fout, "Reply-To: SYSOP.bbs@%s"CRLF, BBSHOST);
	fprintf(fout, "From: SYSOP.bbs@%s"CRLF, BBSHOST);
	fprintf(fout, "To: %s"CRLF, trec->email);
	fprintf(fout, "Subject: @%s@[-%9.9d:%d-]%s mail check."CRLF, trec->userid, (int)code, (int)pid, BBSID);
	fprintf(fout, "X-Purpose: %s registration mail."CRLF, BBSNAME);
	fprintf(fout, "X-Priority: 1 (Highest)"CRLF);
	fprintf(fout, "X-MSMail-Priority: High"CRLF);
	fputs(CRLF, fout);
	
	fprintf(fout, "[中文]"CRLF);
	fprintf(fout, "BBS 地址           : %s (%s)"CRLF, BBSHOST, BBSIP);
	fprintf(fout, "您注册的 BBS ID    : %s"CRLF, trec->userid);
	fprintf(fout, "申请日期           : %s", ctime(&trec->firstlogin));
	fprintf(fout, "登入来源           : %s"CRLF, fromhost);
	fprintf(fout, "您的真实姓名/昵称  : %s (%s)"CRLF, trec->realname, trec->username);
#ifdef CODE_VALID
	strncpy(buf, genrandpwd(pid), sizeof(buf));
	fprintf(fout, "认证暗码           : %s (请注意大小写)"CRLF, buf);
#endif  
	fprintf(fout, "认证信发出日期     : %s"CRLF, ctime(&code));
	fprintf(fout, "登录并点击链接以激活 :   http://argo.sysu.edu.cn/bbsauth?type=1&userid=%s&code=%s "CRLF, trec->userid, buf);

	fprintf(fout, "[English]"CRLF);
	fprintf(fout, "BBS LOCATION       : %s (%s)"CRLF, BBSHOST, BBSIP);
	fprintf(fout, "YOUR BBS USER ID   : %s"CRLF, trec->userid);
	fprintf(fout, "APPLICATION DATE   : %s", ctime(&trec->firstlogin));
	fprintf(fout, "LOGIN HOST         : %s"CRLF, fromhost);
	fprintf(fout, "YOUR NICK NAME     : %s"CRLF, trec->username);
	fprintf(fout, "YOUR NAME          : %s"CRLF, trec->realname);
#ifdef CODE_VALID
	fprintf(fout, "AUTHENTICATION CODE: %s (case sensitive)"CRLF, buf);
	fprintf(fout, "LOGIN AND CLICK THIS LINK TO ACTIVATION YOUR ID :\n http://argo.sysu.edu.cn/bbsauth?type=1&userid=%s&code=%s "CRLF, trec->userid, buf);
#endif
	fprintf(fout, "THIS MAIL SENT ON  : %s"CRLF, ctime(&code));

	fprintf(fout, ".\n");
	fclose(fout);
}



int
get_code(struct userec *trec)
{
	//char username[NAMELEN]; // because I don‘t know the most length of mail name
	char email[STRLEN - 12];
	char *mail, *username;
	struct user_info *u;
	struct userec *x = &currentuser;
	

	username = getparm("user");
	mail = getparm("mail");
	search_ulist(&u, x->userid);
	
	if(strcmp(mail, "sysu.edu.cn") 
	&& strcmp(mail, "student.sysu.edu.cn")
	&& strcmp(mail, "mail.sysu.edu.cn")
	&& strcmp(mail, "mail2.sysu.edu.cn")) {
		http_fatal("错误的邮箱地址");
	}
	sprintf(email, "%s@%s", username, mail);

	//set_safe_record();
	strlcpy(trec->email,email,sizeof(email));
	if (substitute_record(PASSFILE, trec, sizeof (struct userec), u->uid) == -1) {
		http_fatal("系统错误，请联系系统维护员\n");
		return -1;
	}
	if(multi_mail_check(email) >= MAXMAIL){
		http_fatal("警告: 当前邮箱已经激活过多! 请勿再试!");
		return 0;
	}
	send_regmail(trec);
	//http_fatal("你的激活信已经寄出\n");
	hs_init(1);
	hs_setfile("bbscodeok.ptn");
	hs_end();
	return 1;
}


int
countmails(void *uentp_ptr, int unused)
{
	static int totalusers;
	struct userec *uentp = (struct userec *)uentp_ptr;

	if (uentp == NULL) {
		int c = totalusers;

		totalusers = 0;
		return c;
	}
	if (uentp->userlevel & (PERM_WELCOME | PERM_BASIC) && 
                    strcmp(uentp->reginfo, genbuf) == 0)
		totalusers++;
	return 0;
}

int multi_mail_check(char *email)
{
	strcpy(genbuf,email);
	countmails(NULL, 0);
	if (apply_record(PASSFILE, countmails, sizeof (struct userec)) == -1) {
		return 0;
	}
	return countmails(NULL, 0);
}


int
countnetids(void *uentp_ptr, int unused)
{
	static int totalusers;
	struct userec *uentp = (struct userec *)uentp_ptr;

	if (uentp == NULL) {
		int c = totalusers;

		totalusers = 0;
		return c;
	}
	char *at = strchr(uentp->reginfo, '@');
	int n = (at == NULL) ? strlen(genbuf) : at - uentp->reginfo;

	if (uentp->userid[0] != '\0' && 
	    uentp->userlevel & (PERM_WELCOME) && 
	    strncmp(uentp->reginfo, genbuf, n) == 0)
		totalusers++;
	return 0;
}


int multi_netid_check(char *netid)
{
	strcpy(genbuf, netid);
	countnetids(NULL, 0);
	if (apply_record(PASSFILE, countnetids, sizeof (struct userec)) == -1) {
		return 0;
	}
	return countnetids(NULL, 0);
}

