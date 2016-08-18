#include "webbs.h"


static int denymodify;	//equal denynum for add, others for modified

int do_deny() {
	int i; 
	char exp[80], board[80], *userid, *content, path[80], fname[80],buf[256];
	int dt;
	init_all();
   	if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
	strsncpy(board, getparm("board"), 30);
	strsncpy(exp, getparm("exp"), 30);
	dt=atoi(getparm("dt"));
	if (anonyboard(board)) http_fatal("�������޷�����ʵid���");
	if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
	if(!has_BM_perm(&currentuser, board)) http_fatal("����Ȩ���б�����");
	loaddenyuser(board);
	userid=getparm("userid");
   	denymodify=denynum;
	if (userid[0] == 0) {
		deny_form(board);
		return 0;
	}
	if(getuser(userid)==0) http_fatal("�����ʹ�����ʺ�");
	if (!strncmp(userid, "guest", 5)) http_fatal("��guestҲ���Ź�������Ц��");
	strcpy(userid, getuser(userid)->userid);
	for(i=0; i<denynum; i++)
		if(!strcasecmp(denyuser[i].blacklist, userid)) { 
			//http_fatal("���û��Ѿ�����");
			denymodify=i;
			if (!dt) {
				deny_form(board);
				return 0;
			}
		}
	if (!HAS_PERM(PERM_OBOARDS) && dt > 21) http_fatal("�����ֻ�ܷ�21��");
	if(dt<1 || dt>999) http_fatal("�����뱻������(1-999)");
	if(exp[0]==0) http_fatal("���������ԭ��");
	if(denynum>40) http_fatal("̫���˱�����");
	if (denymodify<denynum) {
		sprintf(buf, "boards/%s/%s", board, denyuser[denymodify].filename);
		unlink(buf);
	}
	content=getparm("content");
	for (i=0; i<100; i++) {
		sprintf(fname, "D.%d.A", time(0)+i);
		sprintf(path, "boards/%s/%s", board, fname);
		if (!file_exist(path)) break;
	}
	sprintf(buf, "���ԭ��%s\n������ڣ�%s\nִ���ߣ�%s\n����ߣ�%s\n\n",
		exp, getdatestr(time(0)+86400*dt), currentuser.userid, userid);
	file_append(path, buf);
	sprintf(buf, "����\n\n");
	if (!strlen(content)) 
		sprintf(buf, "(ֱ�ӷ�����޸���)");
	file_append(path, buf);
	file_append(path, content);
	strlcpy(denyuser[denymodify].filename, fname, FNAMELEN);
	strlcpy(denyuser[denymodify].blacklist, userid, IDLEN + 2);
	strlcpy(denyuser[denymodify].executive, currentuser.userid, IDLEN + 2);
	strlcpy(denyuser[denymodify].title, exp, TITLELEN);
	denyuser[denymodify].undeny_time=time(0)+86400*dt;
	if (denymodify==denynum) {
		denynum++;
		savedenyuser(board);
		printf("��� %s �ɹ�<br>\n", userid);
		deny_add_inform(board, userid, exp, dt);
	} else {
		savedenyuser(board);
		printf("���� %s ��������ɹ�<br>", userid);
		deny_modify_inform(board, userid, exp, dt);
	}
	printf("[<a href=bbsdenyall?board=%s>���ر����ʺ�����</a>]", board);
	http_quit();
	return 0;
}

void deny_form(char *board) {
	char buf[256], fname[STRLEN];
	int i;
	FILE *fp;
	printf("<center>%s -- ������� [������: %s]<hr color=green>\n", BBSNAME, board);
	printf("<form action=bbsdenyadd>");
	printf("<input type=hidden name=board value='%s'>", board);
	if (denymodify==denynum) {
		printf("���ʹ����<input name=userid size=12> ");
		printf("����POSTȨ <input name=dt size=2> ��, ");
		printf("ԭ��<input name=exp size=20>\n");
		printf("<input type=submit value=ȷ��>");
		printf("<br>���ģ�<br><textarea name=content rows=20 cols=80 wrap=physicle>");
	} else {
        	printf("���ʹ����<input name=userid size=12 value='%s'> ",
			denyuser[denymodify].blacklist); 
        	printf("����POSTȨ <input name=dt size=2 value=%d> ��, ",
			(denyuser[denymodify].undeny_time-time(0))/86400+1);
        	printf("ԭ��<input name=exp size=20 value='%s'>\n",
			denyuser[denymodify].title);
		printf("<input type=submit value=ȷ��>");
		printf("<br>���ģ�<br><textarea name=content rows=20 cols=80 wrap=physicle>");
		sprintf(fname, "boards/%s/%s", board, denyuser[denymodify].filename);
		fp = fopen(fname, "r");
		if (fp) {
			for (i=0; i<7; i++) fgets(buf, 256, fp);
			while (!feof(fp)) {
				fgets(buf, 256, fp);
				printf("%s", buf);
			}
			fclose(fp);
		}
	}
	printf("</textarea></form>");
}

void deny_modify_inform(char *board, char *user, char *exp, int dt) {
	FILE *fp;
	char path[80], title[80];
	sprintf(title, "�޸Ķ� %s ��ȡ�� %s �淢��Ȩ���Ĵ���", user, board);
	sprintf(path, "tmp/%d.tmp", getpid());
	fp=fopen(path, "w+");
	fprintf(fp, "\n  \033[1;4;32m%s\033[m ����:\n\n", user);
	fprintf(fp, "    �������� \033[1;4;36m%s\033[m ��ȡ�� \033[1;4;33m����\033[m Ȩ�������⣬�ֱ�����£�\n\n", board);
	fprintf(fp, "    �����ԭ��\033[1;4;33m%s\033[m\n\n", exp);
	fprintf(fp, "    �����ڿ�ʼ��ֹͣ��Ȩ��ʱ�䣺 \033[1;4;35m%d\033[m ��\n\n", dt);
	fprintf(fp, "    ������ \033[1;4;35m%s\033[m �� \033[1;4;32m%s\033[m ���������⡣\n\n",
		getdatestr(time(0)+dt*86400), currentuser.userid);
	fclose(fp);
	securityreport(title);
	if (!anonyboard(board))
		post_inform(board, title, path);
	post_mail(user, title, path, currentuser.userid, currentuser.username, 
		fromhost, -1, 0);
	unlink(path);
	printf("ϵͳ�Ѿ�����֪ͨ��%s.<br>\n", user);
}

char *getdatestr(time_t now)
{
        struct tm *tm;
	static char buf[80];

        tm = localtime(&now);
	sprintf(buf, "%4d��%02d��%02d��",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
	return buf;
}

int deny_list() {
	int i; 
	char board[80];
	init_all();
   	if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
	strsncpy(board, getparm("board"), 30);
	if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
	if(!has_BM_perm(&currentuser, board)) http_fatal("����Ȩ���б�����");
	loaddenyuser(board);
   	printf("<center>\n");
   	printf("%s -- �����û����� [������: %s]<hr color=green><br>\n", BBSNAME, board);
   	printf("���湲�� %d �˱���<br>", denynum);
   	printf("<table border=1><tr><td>���<td>�û��ʺ�<td>����ԭ��<td>ִ����<td>ԭ���������<td>����\n");
   	for(i=0; i<denynum; i++) {
		printf("<tr><td>%d", i+1);
		if (!anonyboard(board))
			printf("<td><a href=bbsqry?userid=%s>%s</a>", 
				denyuser[i].blacklist, denyuser[i].blacklist);
		else printf("<td>%s", board);
		printf("<td><a href=bbsdenyattach?userid=%s&board=%s>%s</a>", 
			denyuser[i].blacklist, board, nohtml(denyuser[i].title));
		printf("<td>%s\n", denyuser[i].executive);
		printf("<td>%s\n", getdatestr(denyuser[i].undeny_time));
		if (!anonyboard(board))
			printf("<td>[<a href=bbsdenyadd?board=%s&userid=%s>�޸�</a>]",
				board, denyuser[i].blacklist);
		printf("[<a onclick='return confirm(\"ȷʵ�����?\")' href=bbsdenydel?board=%s&num=%d>���</a>]", 
			board, i);
	}
   	printf("</table><hr color=green>\n");
	printf("<table class=foot><th class=foot>");
	printf("<a href=bbsdenyadd?board=%s>�趨�µĲ���POST�û�</a>\n", board);
	printf("</table></center>");
	http_quit();
	return 0;
}
        
int deny_attach() {
	int i;
	char *userid, board[80], buf[512];
	FILE *fp;
        init_all();
        if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
        strsncpy(board, getparm("board"), 30);
        if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
        if(!has_BM_perm(&currentuser, board)) http_fatal("����Ȩ���б�����");
        loaddenyuser(board);
        userid=getparm("userid");
        if (userid[0] == '\0') return -1;
        if (getuser(userid) == NULL) http_fatal("�����ʹ�����ʺ�");   
        strcpy(userid, getuser(userid)->userid);
        printf("<center>%s -- ������� [������: %s]<hr color=green>", BBSNAME, board);
	for (i=0; i<denynum; i++) {
		if (!strcasecmp(denyuser[i].blacklist, userid)) {
			sprintf(buf, "boards/%s/%s", board, denyuser[i].filename);		
			fp=fopen(buf, "r");
			if (fp<=0) http_fatal("���Ĳ����ڻ����ѱ�ɾ��");
			printf("<table class=doc width=610 border=1 style='BORDER: 2px solid; BORDER-COLOR: D0F0C0;'>\n");
        		printf("<tr><td class=doc>\n<pre>");
			while (1) {
				if (fgets(buf, 512, fp)==0) break;
				hhprintf("%s", void1(buf));
			}
			fclose(fp);
        		printf("</pre>\n</table><hr>\n");
			if (!anonyboard(board)) 
				printf("<td>[<a href=bbsdenyadd?board=%s&userid=%s>�޸�</a>]",
					board, denyuser[i].blacklist);
        		printf("<td>[<a onclick='return confirm(\"ȷʵ�����?\")' href=bbsdenydel?board=%s&num=%d>���</a>]",
				board, i);
			printf("</center>");
			http_quit();
		}
	}
	http_fatal("���û�û�б���");
	return 0;
}

int do_undeny() {
	int num; 
	char board[STRLEN], userid[STRLEN], path[STRLEN];
	init_all();
   	if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
	strsncpy(board, getparm("board"), 30);
	if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
	if(!has_BM_perm(&currentuser, board)) http_fatal("����Ȩ���б�����");
	loaddenyuser(board);
	num=atoi(getparm("num"));
	if (num<0 || num>=denynum) http_fatal("����û����ڱ���������");
	strlcpy(userid, denyuser[num].blacklist, IDLEN + 1);
	denyuser[num].blacklist[0]=0;
	savedenyuser(board);
	sprintf(path, "boards/%s/%s", board, denyuser[num].filename);
	unlink(path);
	printf("�Ѿ��� %s ���. <br>\n", anonyboard(board)?"���û�":userid);
	undeny_inform(board, userid);
	printf("[<a href=bbsdenyall?board=%s>���ر�������</a>]", board);
	http_quit();
	return 0;
}

int m_deny() {
/* henry: ʵ�ְ�������Ŷ����߷���Ĺ��� */
//        FILE *fp;
	int fd;
        char board[STRLEN], dir[STRLEN], *userid, buf[256], fname[STRLEN], path[STRLEN];
	char exp[STRLEN];
	int dt, num;
        struct boardheader *x1;
        struct fileheader x;
        int i, total;
        init_all();
        strsncpy(board, getparm("board"), 32);
        strsncpy(exp, getparm("exp"), 30); 
        dt=atoi(getparm("dt"));
        x1=getbcache(board);
        if(x1==0) http_fatal("�����������");
        strcpy(board, x1->filename);
        if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
        if(!has_BM_perm(&currentuser, board)) http_fatal("��û��Ȩ�޽��б�����");
	setboardfile(dir, board, ".DIR");
	fd = open(dir, O_RDONLY);
	if (fd == -1) http_fatal("�����������Ŀ¼");
        total=file_size(dir)/sizeof(struct fileheader);
        num=atoi(getparm("num"));
        if(num<0 || num>total-1) http_fatal("�����ڱ�����");
        if(total<=0) http_fatal("��������Ŀǰû������");
	lseek(fd, num*sizeof(struct fileheader), SEEK_SET);
	if (read(fd, &x, sizeof(x)) == -1)
		http_fatal("�����ڱ�����");
	close(fd);
	userid=x.owner;
	if (anonyboard(board)) userid += IDLEN+2;
        if (getuser(userid)==0) http_fatal("�����ʹ�����ʺ�");
        strcpy(userid, getuser(userid)->userid);
	loaddenyuser(board);
        for(i=0; i<denynum; i++)
                if(!strcasecmp(denyuser[i].blacklist, userid)) {
                        http_fatal("���û��Ѿ�����");
                }
	if (exp[0]==0) {
		m_deny_add_form(board, userid, num);
		return 0;
	}
        if(dt<1 || dt>999) http_fatal("�����뱻������(1-999)");
        if(denynum>40) http_fatal("̫���˱�����");
	if (!HAS_PERM(PERM_OBOARDS) && dt > 21) http_fatal("�����ֻ�ܷ�21��");
        for (i=0; i<100; i++) {
                sprintf(fname, "D.%d.A", time(0)+i);
		setboardfile(path, board, fname);
                if (!file_exist(path)) break;
        }
	setboardfile(dir, board, x.filename);
	fd = open(dir, O_RDONLY);
	if (fd == -1) http_fatal("�޷��򿪸�����");
        sprintf(buf, "���ԭ��%s\n������ڣ�%s\nִ���ߣ�%s\n",
                exp, getdatestr(time(0)+86400*dt), currentuser.userid);	
        file_append(path, buf);
	if (!anonyboard(board)) {
		sprintf(buf, "����ߣ�%s\n", userid);
		file_append(path, buf);
	}
        sprintf(buf, "\n����\n\n");
	file_append(path, buf);
	while (1) {
		if (read(fd, buf, sizeof(buf)) == 0) break;
		if(!strncmp(buf, "--\n", 3)) break;
		file_append(path, buf);
	}
	close(fd);

        strcpy(denyuser[denynum].filename, fname);
        strsncpy(denyuser[denynum].blacklist, userid, 13);
        strsncpy(denyuser[denynum].executive, currentuser.userid, 13);
        strsncpy(denyuser[denynum].title, exp, 30);	
        denyuser[denynum].undeny_time=time(0)+86400*dt;
	denynum++;
	savedenyuser(board);
	deny_add_inform(board, userid, exp, dt);
        printf("��� %s �ɹ�<br>\n", anonyboard(board)?"���û�":userid);
	printf("[<a href=bbsmdoc?board=%s>���ع���ģʽ</a>]", board);
	http_quit();
	return 0;
}

void m_deny_add_form(char *board, char *userid, int num) {
        printf("<center>%s -- ������� [������: %s]<hr color=green>\n", BBSNAME, board);
        printf("<form action=bbsdenymadd>");
        printf("<input type=hidden name=board value='%s'>", board);
	printf("<input type=hidden name=num value='%d'>", num);
	if (anonyboard(board)) printf("���������ʹ����");
	else printf("���ʹ���� %s ", userid);
	printf("����POSTȨ <input name=dt size=2> ��, ");
        printf("ԭ��<input name=exp size=20>\n");
        printf("<input type=submit value=ȷ��>");
        printf("</textarea></form>");
}

void deny_add_inform(char *board, char *user, char *exp, int dt) {
	FILE *fp;
	int fd;
	char path[80], title[80];
	sprintf(title, "%s ��ȡ���� %s ��ķ���Ȩ��", user, board);
	strcpy(path, "tmp/deny.XXXXXX");
	fd = mkstemp(path);
	if (fd == -1) http_fatal("���ʧ��");
	fp = fdopen(fd, "w");
	if (fp == NULL) http_fatal("���ʧ��");

	fprintf(fp, "\n  \033[1;32m%s\033[m ����: \n\n", user);
	fprintf(fp, "    ���Ѿ��� \033[1;32m%s\033[m ȡ���� \033[1;4;36m%s\033[m ��ķ���Ȩ�� \033[1;35m%d\033[m �졣\n\n", currentuser.userid, board, dt);
	fprintf(fp, "    ���������ԭ����: \033[1;4;33m%s\033[m\n\n", exp);
	fprintf(fp, "    ������ \033[1;35m%14.14s\033[m ��ý�⡣�������ʣ���鿴\033[1;33mվ����\033[m��ز��֡�\n\n", getdatestr(time(0)+dt*86400));
	fprintf(fp, "    �������飬�뵽 \033[1;4;36mComplain\033[m �水��ʽ������ߣ���л������\n\n");

	fclose(fp);

	securityreport(title);
	if (!anonyboard(board))
		post_inform(board, title, path);
	post_mail(user, title, path, currentuser.userid, currentuser.username, 
		fromhost, -1, 0);
	unlink(path);
	printf("ϵͳ�Ѿ�����֪ͨ��%s.<br>\n", anonyboard(board)?"���û�":user);
}

int m_undeny() {
	int num; 
	char board[80], userid[80], path[80];
	init_all();
   	if(!loginok) http_fatal("����δ��¼, ���ȵ�¼");
	strsncpy(board, getparm("board"), 30);
	if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
	if(!has_BM_perm(&currentuser, board)) http_fatal("����Ȩ���б�����");
	loaddenyuser(board);
	num=atoi(getparm("num"));
	if (num<0 || num>=denynum) http_fatal("����û����ڱ���������");
	strlcpy(userid, denyuser[num].blacklist, IDLEN + 1);
	denyuser[num].blacklist[0]=0;
	savedenyuser(board);
	sprintf(path, "boards/%s/%s", board, denyuser[num].filename);
	unlink(path);
	printf("�Ѿ��� %s ���. <br>\n", anonyboard(board)?"���û�":userid);
	undeny_inform(board, userid);
	printf("[<a href=bbsdenyall?board=%s>���ر�������</a>]", board);
	http_quit();
	return 0;
}

void undeny_inform(char *board, char *user) {
        FILE *fp;
	int fd;
        char path[80], title[80];
        sprintf(title, "�ָ� %s �� %s ��ķ���Ȩ��", user, board);
	
	strcpy(path, "tmp/undeny.XXXXXX");
	fd = mkstemp(path);
	if (fd == -1) http_fatal("���ʧ��");
	fp = fdopen(fd, "w");
	if (fp == NULL) http_fatal("���ʧ��");
        fprintf(fp, "\n  \033[1;4;32m%s\033[m ���ѣ�\n\n", user);
	fprintf(fp, "    ����ʱ���ѹ����ָֻ����� \033[1;4;36m%s\033[m ��� \033[1;4;33m����\033[m Ȩ����", 
		board);
        fclose(fp);
	if (!anonyboard(board)) 
        	post_inform(board, title, path);
        post_mail(user, title, path, currentuser.userid, currentuser.username, 
		fromhost, -1, 0);
        unlink(path);
        printf("ϵͳ�Ѿ�����֪ͨ��%s.<br>\n", anonyboard(board)?"���û�":user);
}


int m_show_board() {
	char board[80];
	struct boardheader *x1;
	struct fileheader x;
	int i, start, total, fd;
	char fname[80];
 	init_all();
	strsncpy(board, getparm("board"), 32);
	x1=getbcache(board);
	if(x1==0) http_fatal("�����������");
	strcpy(board, x1->filename);
	if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
	if(!has_BM_perm(&currentuser, board)) http_fatal("��û��Ȩ�޷��ʱ�ҳ");
	setboardfile(fname, board, ".DIR");
	fd = open(fname, O_RDONLY);
	if (fd == -1) http_fatal("�����������Ŀ¼");
        total=file_size(fname)/sizeof(struct fileheader);
	start=atoi(getparm("start"));
        if(strlen(getparm("start"))==0 || start>total-20) start=total-20;
  	if(start<0) start=0;
	printf("<nobr><center>\n");
	printf("%s -- [������: %s] ����[%s] ������[%d]<hr color=green>\n", 
		BBSNAME, board, userid_str(x1->BM), total);
	if (total<=0) http_fatal("��������Ŀǰû������");
	printf("<form name=form1 method=post action=bbsman>\n");
      	printf("<table width=613>\n");
      	printf("<tr><td>���<td>����<td>״̬<td>����<td>����<td>����<td>���\n");
	lseek(fd, start*sizeof(struct fileheader), SEEK_SET);
      	for(i=0; i<20; i++) {
		if (read(fd, &x, sizeof(x)) == 0) break;
		printf("<tr><td>%d", start+i+1);
		printf("<td><input style='height:18px' name=box%s type=checkbox>", x.filename);
		printf("<td>%s<td>%s", flag_str(x.flag), userid_str(x.owner));
         	printf("<td>%12.12s", Cdtime(atoi(x.filename+2)));
         	printf("<td><a href=bbscon?board=%s&file=%s&start=%d>%s%36.36s </a>",
			board, x.filename, start+i,
			strncmp(x.title, "Re: ", 4) ? "�� " : "",
			void1(nohtml(x.title)));
		printf("<td><a href=bbsdenymadd?board=%s&num=%d>������û�</a>",
			board, start+i);
      	}
	close(fd);
      	printf("</table>\n");
	printf("<input type=hidden name=mode value=''>\n");
	printf("<input type=hidden name=board value='%s'>\n", board);
	printf("<input type=button value=ɾ�� onclick='if (confirm(\"ɾ�������½��޷��ָ��������Ҫɾ����?\")) {document.form1.mode.value=1; document.form1.submit();}'>\n");
	printf("<input type=button value=��M onclick='document.form1.mode.value=2; document.form1.submit();'>\n");
	printf("<input type=button value=��G onclick='document.form1.mode.value=3; document.form1.submit();'>\n");
	printf("<input type=button value=����Re onclick='document.form1.mode.value=4; document.form1.submit();'>\n");
	printf("<input type=button value=���MG onclick='document.form1.mode.value=5; document.form1.submit();'>\n");
	printf("</form>\n");
	if(start>0) {
		printf("<a href=bbsmdoc?board=%s&start=%d>��һҳ</a> ",
			board, start<20 ? 0 : start-20);
	}
	if(start<total-20) {
		printf("<a href=bbsmdoc?board=%s&start=%d>��һҳ</a> ",
			board, start+20);
	}
	printf("<a href=bbsdoc?board=%s>һ��ģʽ</a> ", board);
	printf("<a href=bbsdenyall?board=%s>��������</a> ", board);
	printf("<a href=bbsmnote?board=%s>�༭���滭��</a> ", board);
	printf("<form action=bbsmdoc?board=%s method=post>\n", board);
	printf("<input type=submit value=��ת��> �� <input type=text name=start size=4> ƪ");
	printf("</form>\n");
	http_quit();
	return 0;
}

int board_manage() {
	int i, total=0, mode;
	char board[BFNAMELEN];
	struct boardheader *brd;
	init_all();

	if(!loginok) http_fatal("���ȵ�¼");
	strlcpy(board, getparm("board"), sizeof(board));
	mode=atoi(getparm("mode"));
	brd=getbcache(board);
	if(brd==0) http_fatal("�����������");
	strlcpy(board, brd->filename, BFNAMELEN);
	if(!has_BM_perm(&currentuser, board)) http_fatal("����Ȩ���ʱ�ҳ");
	if(mode<=0 || mode>5) http_fatal("����Ĳ���");
	printf("<table>");
	for(i=0; i<parm_num && i<40; i++) {
		if(!strncmp(parm_name[i], "box", 3)) {
			total++;
			char *file = parm_name[i]+3;
			if(mode==1) {
				if (do_del(board, file) == 0) {
					BBS_SINGAL("/post/del",
						   "b", board,
						   "f", file,
						   NULL);
					BBS_SINGAL("/post/cancelpost",
						   "b", board,
						   "f", file,
						   NULL);
				}
			}
			if(mode==2) {
				if (do_set(board, file, FILE_MARKED) == 0) {
					BBS_SINGAL("/post/changemark",
						   "f", file,
						   "b", board,
						   "m", "1",
						   NULL);
				}
			}
			if(mode==3) {
				if (do_set(board, file, FILE_DIGEST) == 0) {
					BBS_SINGAL("/post/changedigest",
						   "f", file,
						   "b", board,
						   "g", "1",
						   NULL);
				}
			}
			if(mode==4) do_set(board, file, FILE_NOREPLY);
			if(mode==5) {
				if (do_set(board, file, 0) == 0) {
					BBS_SINGAL("/post/changemark",
						   "f", file,
						   "b", board,
						   "m", "0",
						   NULL);
					BBS_SINGAL("/post/changedigest",
						   "f", file,
						   "b", board,
						   "g", "0",
						   NULL);
				}
			}
		}
	}
	printf("</table>");
	if(total<=0) printf("����ѡ������<br>\n");
	printf("<br><a href=bbsmdoc?board=%s>���ع���ģʽ</a>", board);
	http_quit();
	return 0;
}

int do_del(char *board, char *file) {
	int fd, num;
	char fname[STRLEN], buf[256];
	struct fileheader f;
	struct userec *u;
	setboardfile(fname, board, ".DIR");
	fd = open(fname, O_RDONLY);
	if (fd == -1) http_fatal("����Ĳ���");
	num = 1;
	while(1) {
		if (read(fd, &f, sizeof(struct fileheader)) == 0) break;
		if(!strcmp(f.filename, file)) {
			if (delete_record(fname, sizeof(struct fileheader), num) == -1)
				http_fatal("ɾ��ʧ��");

			snprintf(buf, sizeof(buf), "%s %-12s delete %-48.48s on %s\n", Cdtime(time(0)), 
				currentuser.userid, f.title, board);
			file_append("wwwlog/trace", buf);

			/* Henry: record delete operator */
			sprintf(buf, "%-32s - %s", f.title, currentuser.userid);
			strlcpy(f.title, buf, 48);
			f.flag = 0;
			if (is_owner(&currentuser, &f)) {
				setboardfile(fname, board, ".JUNK");
			} else {
				setboardfile(fname, board, ".DELETED");
			}
			append_record(fname, &f, sizeof(struct fileheader));
			printf("<tr><td>%s  <td>����:%s <td>ɾ���ɹ�.\n", f.owner, nohtml(f.title));
			u=getuser(f.owner);
			if(!junkboard(board) && u) {
				if(u->numposts>0) u->numposts--;
				save_user_data(u);
			}
			close(fd);
			return 0;
		}
		num++;
	}
	close(fd);
	printf("<tr><td><td>%s<td>�ļ�������.\n", file);
	return -1;
}

int do_set(char *board, char *file, int flag) {
	FILE *fp;
	char path[256], dir[256];
	struct fileheader f;
	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp = fopen(dir, "r+");
	if (fp == NULL) http_fatal("����Ĳ���");
	while(1) {
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0) break;
		if(!strcmp(f.filename, file)) {
			f.flag|=flag;
			if(flag==0) {
				if (f.flag & FILE_DIGEST)
					do_unset_digest(board, &f);
				f.flag=0;
			}
			fseek(fp, -1*sizeof(struct fileheader), SEEK_CUR);
			fwrite(&f, sizeof(struct fileheader), 1, fp);
			fclose(fp);
			printf("<tr><td>%s<td>����:%s<td>", f.owner, nohtml(f.title));
			if (flag == FILE_DIGEST && do_set_digest(board, &f) < 0) {
				printf("���ʧ��.\n");
				return -1;
			} else {
				printf("��ǳɹ�.\n");
				return 0;
			}
		}
	}
	fclose(fp);
	printf("<td><td><td>%s<td>�ļ�������.\n", file);
	return -1;
}

int do_set_digest(char *board, struct fileheader *fptr) {
	char file[80], fname[80];

	snprintf(fname, sizeof(fptr->filename), "G%s", fptr->filename+1);
	setboardfile(file, board, fname);
	if (file_exist(file)) return 0;	//This file was added to digest before

	setboardfile(fname, board, fptr->filename);
	if (f_cp(fname, file, 0) < 0) return -1;
	
	/* Henry: Clear FILE_DIGEST & FILE_MARKED for digest file */
	fptr->flag = 0;
	snprintf(fname, sizeof(fptr->filename), "G%s", fptr->filename+1);
	strncpy(fptr->filename, fname, sizeof(fptr->filename));
	setboardfile(fname, board, ".DIGEST");
	if (append_record(fname, fptr, sizeof(struct fileheader)) == -1) {
		unlink(file);
		return -1;
	}
	return 0;
}

int do_unset_digest(char *board, struct fileheader *fptr) {
	int n;
	struct fileheader header;
	char direct[80], file[80], fname[80];

	snprintf(fname, sizeof(fname), "G%s", fptr->filename + 1);
	setboardfile(file, board, fname);
	unlink(file);
	setboardfile(direct, board, ".DIGEST");
	n = search_record(direct, &header, sizeof(header), cmpfilename, fname);
	if (n) return delete_record(direct, sizeof(header), n);
	return 0;
}

int change_note() {
	FILE *fp;
	char *ptr, path[256], buf[10000], board[256];
   	init_all();
	printf("<center>\n");
	if(!loginok) http_fatal("�Ҵҹ��ͣ����ȵ�¼");
	strsncpy(board, getparm("board"), 30);
	if(!has_BM_perm(&currentuser, board)) http_fatal("����Ȩ���б�����");
	strsncpy(board, getbcache(board)->filename, 30);
	sprintf(path, "vote/%s/notes", board);
	if(!strcasecmp(getparm("type"), "update")) save_note(path);
	printf("%s -- �༭���滭�� [������: %s]<hr>\n", BBSNAME, board);
   	printf("<form method=post action=bbsmnote?type=update&board=%s>\n", board);
	fp=fopen(path, "r");
	if(fp) {
		fread(buf, 9999, 1, fp);
		ptr=strcasestr(buf, "<textarea>");
		if(ptr) ptr[0]=0;
		fclose(fp);
	}
   	printf("<table width=610 border=1><tr><td>");
   	printf("<textarea name=text rows=20 cols=80 wrap=physicle>\n");
	printf("%s", void1(buf));
   	printf("</textarea></table>\n");
   	printf("<input type=submit value=����> ");
   	printf("<input type=reset value=��ԭ>\n");
   	printf("<hr>\n");
	http_quit();
	return 0;
}

int save_note(char *path) {
	FILE *fp;
	char buf[10000];
	fp=fopen(path, "w");
	if (fp == NULL) return -1;
	strsncpy(buf, getparm("text"), 9999);
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("���滭���޸ĳɹ���<br>\n");
	printf("<a href='javascript:history.go(-2)'>����</a>");
	http_term();
	return 0;
}

int create_board() {
	struct boardheader *bhptr = NULL, bh;
	char board[BFNAMELEN + 1], desc[BTITLELEN + 1], bm[BMLEN + 1];
	char type[5], sec[2];
	char fname[STRLEN];
	int id;
	int newflag;

	init_all();
	if (!HAS_PERM(PERM_SYSOP)) http_fatal("����Ȩ���ʴ�ҳ");
	strlcpy(board, getparm("board"), sizeof(board));
	if (board[0]) bhptr = getbcache(board);
	strlcpy(desc, getparm("desc"), sizeof(desc));
	strlcpy(bm, getparm("bm"), sizeof(bm));
	strlcpy(type, getparm("type"), sizeof(type));
	strlcpy(sec, getparm("sec"), sizeof(sec));

	newflag = (bhptr == NULL);
	if (board[0] == '\0' || desc[0] == '\0' || type[0] == '\0'
		|| sec[0] == '\0') {
		show_brdctl_form(bhptr);
		http_quit();
	}


	if (newflag) {		//�°�
		sprintf(fname, "boards/%s", board);
		f_mkdir(fname, 0644);
		sprintf(fname, "0Announce/boards/%s", board);
		f_mkdir(fname, 0644);
		memset(&bh, 0, sizeof(bh));
		strlcpy(bh.filename, void1(board), BFNAMELEN + 1);
		snprintf(bh.title, BTITLELEN, "%1.1s[%4.4s] �� %s",
			sec, type, desc);
		if (bm[0])
			strlcpy(bh.BM, bm, BMLEN + 1);

		append_record(".BOARDS", &bh, sizeof(bh));
		printf("%s�洴���ɹ�", board);
	} else {
		id = search_record(".BOARDS", &bh, sizeof(bh), cmpboardname, board);
		if (id == 0) http_fatal("%s�����ʧ��", board);
		snprintf(bh.title, BTITLELEN, "%1.1s[%4.4s] �� %s",
			sec, type, desc);
		if (bm[0])
			strlcpy(bh.BM, bm, BMLEN + 1);
		substitute_record(".BOARDS", &bh, sizeof(bh), id);
		printf("%s���޸ĳɹ�", board);
	}
	//update_bcache();
	return 0;
}

int show_brdctl_form(struct boardheader *bhptr) {
	printf( "<form action=bbsbrdcreate>\n"
		"������<input name=board value=%s><br>\n"
		"��������������<input name=desc valus=%s><br>\n"
		"���<input name=type value=%4.4s><br>\n"
		"��������<input name=sec value=%c><br>\n"
		"������<input name=bm value=%s><br>\n"
		"<input type=submit value=ȷ��>\n",
		bhptr?bhptr->filename:"", bhptr?bhptr->title + 11:"",
		bhptr?bhptr->title + 2:"ϵͳ",
		bhptr?bhptr->title[0]:'a',
		bhptr?bhptr->BM:"");
	http_quit();
	return 0;
}

int edit_pattern() {
	char *fname;
	char file[80];

	init_all();
	if (!HAS_PERM(PERM_SYSOP)) http_fatal("����Ȩ���ʴ�ҳ");
	fname = getparm("fname");
	if (strlen(fname) == 0 || strstr(fname, "..")) http_fatal("�ļ�������");
	snprintf(file, sizeof(file), "pattern/%s.ptn", fname);
	if (!file_exist(file)) http_fatal("�ļ�������");
	hs_init(10);
	hs_setfile("bbseditptn.ptn");
	hs_assign("PTNAME", fname);
	hs_assign("PATTERN", file);
	hs_end();
	http_quit();
	return 0;
}

int modify_pattern() {
	char *fname, *source, file[80];
	int fd;

	init_all();
	if (!HAS_PERM(PERM_SYSOP)) http_fatal("����Ȩ���ʴ�ҳ");
	fname = getparm("fname");
	source = getparm("source");
	if (strlen(fname) == 0 || strstr(fname, "..")) http_fatal("�ļ�������");
	snprintf(file, sizeof(file), "pattern/%s.ptn", fname);
	if (!file_exist(file)) http_fatal("�ļ�������");
	if (strlen(source) == 0) http_fatal("����дģ���ļ�����");

	fd = open(file, O_WRONLY);
	if (fd < 0) http_fatal("���ļ�����");
	truncate(file, 0);
	if (write(fd, source, strlen(source)) < 0) http_fatal("�޷�д���ļ�");
	close(fd);
	printf("ģ�� %s ���³ɹ�", fname);
	return 0;
}

int restart_httpd() {
	init_all();
	if (!HAS_PERM(PERM_SYSOP)) http_fatal("����Ȩ���ʸ�ҳ");

	printf("%d", getppid());
	kill(getppid(), SIGTERM);
#ifdef SET_EFFECTIVE_ID	
		if (getuid() == 0) {
			seteuid(0);
			if (geteuid() != 0) {
				http_fatal("����ʧ��");
			}
		}
#endif
	if (fork() == 0) {
		char port[10];
		snprintf(port, sizeof(port), "%d", server_port);
		printf(BBSHOME"/bin/httpd %s", port);
		close(0);
		close(1);
		close(2);
		execl(BBSHOME"/bin/httpd", BBSHOME"/bin/httpd", port, NULL);
		http_fatal("����httpdʧ��");
	}

	printf("httpd������...done");
	return 0;
}
