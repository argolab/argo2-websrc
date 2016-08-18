#include "webbs.h"
//#include "../include/prototypes.h"

const char *getfile(char *filepath)
{
	struct stat st;
	if ( stat(filepath, &st) < 0) return "";
	static char buf[2048];

	int fd = open(filepath, O_RDONLY, 0600);
	if ( fd < 0 )	 return "";
	read(fd, buf, st.st_size);
	close(fd);
	return buf;

}

static unsigned  int ip_str2int(const char *ip)
{
	/* Assume ip is ipv4 : a.b.c.d */
	int dotcnt = 0, i;
	for (i = 0; ip[i]; i++)
	{
		if ( ip[i] == '.' ) dotcnt++;
	}
	if ( dotcnt != 3 ) return 0;
	unsigned int ret = 0;
	char buf[256];
	strlcpy(buf, ip, sizeof( buf ));
	char *tk = strtok(buf, ".");
	while ( tk != NULL )
	{
		ret = ret * 256 + atoi(tk);
		tk = strtok(NULL, ".");
	}
	return ret;
}

int check_outcampus_ip()
{
	// If ip_of_sysu.lst not exists
	// Seen as not outcampus
	if (access(SYSU_IP_LIST, F_OK) == -1) {
		return NA;
	}

	FILE *fp = fopen(SYSU_IP_LIST, "r");
	char ip_start[64], ip_end[64];
	unsigned int ip_s, ip_e;
	unsigned int ip_now = ip_str2int(fromhost);

	if ( fp == NULL ) return NA;
	while ( fscanf( fp, "%s%s", ip_start, ip_end ) != EOF ) {
		ip_s = ip_str2int(ip_start);
		ip_e = ip_str2int(ip_end);
		if ( ip_now >= ip_s && ip_now <= ip_e ) {
			fclose(fp);
			return NA;
		}
	}
	fclose(fp);
	return YEA;
}

int write_article() 
{
   	FILE *fp;
   	int i;
	char userid[IDLEN + 1], buf[512], path[512];
	char file[FNAMELEN + 1]="", board[BFNAMELEN + 1], title[TITLELEN + 1]="";
	char title_buf[TITLELEN * 6 + 1];
	int noreply = 0;
	struct fileheader *fileinfo = NULL;
	struct boardheader *bp;
	char content[2048];
	int articleid = 0;

   	init_all();
	modify_user_mode(POSTING);
	strlcpy(board, getparm("board"), sizeof(board));
	strlcpy(file, getparm("file"), sizeof(file));
	strlcpy(userid, getparm("userid"), sizeof(userid));
	if(file[0]!='M' && file[0]) http_fatal("������ļ���");
	bp = getbcache(board);
	
	if ( ! (bp->flag & BRD_RESTRICT) 
			//&& !has_BM_perm(&currentuser, board)
			&& YEA == check_outcampus_ip()) {
			const char *msg = getfile(BAN_OUTIP_ANN);
			http_fatal(msg);
	}

	if (bp == NULL) 
		http_fatal("�������������������Ȩ�ڴ���������������");

	strlcpy(board, bp->filename, sizeof(board));
	if(file[0]){
 		fileinfo=get_file_ent(board, file);
		if (fileinfo == NULL) 
			http_fatal("������ļ�����");
		noreply = fileinfo->flag & FILE_NOREPLY ||bp->flag & NOREPLY_FLAG;
		if(!(!noreply || HAS_PERM(PERM_SYSOP) ||has_BM_perm(&currentuser, board)
			|| is_owner(&currentuser, fileinfo)))
			http_fatal("�Բ���, �������в��� RE ����, �㲻�ܻظ�(RE) ��ƪ����.");
		strcpy(title, fileinfo->title);
        	if(title[0] && strncmp(title, "Re: ", 4)) {
			snprintf(buf, sizeof(buf), "Re: %s", title);
			strlcpy(title, buf, TITLELEN);
		}	/* Henry: to compat with other version of *NIX */
	/* by Henry 2002.12.7
	��һƪ���µ�article id�����ļ����м��Ǹ����֣�
	re�ĺ��������ȵ�article idһ����
	*/
		articleid = fileinfo->id;
	}

	strcpy(content, "\n\n");
	int conlen = sizeof(content) - strlen(content);
	hs_init(20);
	hs_setfile("bbspst.ptn");
	snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "%d", getsecnum(board));
	hs_assign("SECNUM", hs_genbuf[0]);
	hs_assign("SECNAME", getsecname(board));
	hs_assign("BOARDNAME", bp->title + 11);
	hs_assign("BOARD", board);
	my_ansi_filter(title);
	strlcpy(title_buf, void1(nohtml(title)), sizeof(title_buf));
	hs_assign("TITLE", title_buf);
	if (!loginok) hs_assign("NOLOGIN", "1");
	if (anonyboard(board)) hs_assign("ANONYBOARD", "1");
	snprintf(hs_genbuf[1], sizeof(hs_genbuf[0]), "%d", articleid);
	hs_assign("ARTICLEID", hs_genbuf[1]);
	if (file[0]) {
		hs_assign("REPLY", "1");
		hs_assign("REFILE", file);
	}
#ifdef ATTACH_UPLOAD
	if (bp->flag & BRD_ATTACH) hs_assign("UPLOADATTACH", "1");
#endif

	if(file[0]) {
		int lines=0;
		char *quser, *ptr;
		setboardfile(path, board, file);
		fp=fopen(path, "r");
		/* urec = getuser(userid); */
		if (fp && fgets(buf, 500, fp) != 0) {
			if ((ptr = strrchr(buf, ')')) != NULL) {
				ptr[1] = '\0';
				if ((ptr = strchr(buf, ':')) != NULL) {
					quser = ptr + 1;
					while (*quser == ' ')
						quser++;
				}
			}
			sprintf(hs_genbuf[2], "�� �� %-.55s �Ĵ������ᵽ: ��\n", 
				quser);
		} else {
			sprintf(hs_genbuf[2], "�� �� %s �Ĵ������ᵽ: ��\n", userid);
		}
		safe_strcat(content, hs_genbuf[2], 0, &conlen);
		/* setboardfile(path, board, file); */
		/* fp=fopen(path, "r"); */
		if(fp) {
			for(i=0; i<3; i++)
				if(fgets(buf, 500, fp)==0) break;
			while(1) {
				if (fgets(buf, 500, fp) == NULL) break;
				if (!strncmp(buf, ": ��", 4)) continue;
				if (!strncmp(buf, ": : ", 4)) continue;
				if (!strncmp(buf, "--\n", 3)) break;
				if (buf[0]=='\n') continue;
				if (lines++>=20) {
					safe_strcat(content, ": (��������ʡ��...)\n", 0, &conlen);
					break;
				}
				safe_strcat(content, ": ", 0, &conlen);
				safe_strcat(content, nohtml(buf), 0, &conlen);
//				if(!strcasestr(buf, "</textarea>")) printf(": %s", buf);
			}
			fclose(fp);
		}
	}
	while (content[strlen(content) - 1] == '\n') content[strlen(content) - 1] = '\0';
	hs_assign("CONTENT", content);
	hs_end();
	http_quit();
	return 0;
}

int submit_article() {
	char filename[80], dir[80], board[80], title[80], buf[80], *content;
	char filepath[80];
	char oldtitle[80];
	char *refile;
	int r, i, sig;
	struct boardheader *brd;
	struct fileheader *fileinfo = NULL;
	int fd;
	
	init_all();
	//if(!loginok) http_fatal("�Ҵҹ��Ͳ��ܷ������£����ȵ�¼");
   	strsncpy(board, getparm("board"), BFNAMELEN);
	board[strlen(board)] = '\0';
   	strsncpy(title, getparm("title"), TITLELEN);
	title[strlen(title)] = '\0';
	strsncpy(oldtitle, getparm("oldtitle"), TITLELEN);

	/* Henry: ����ǩ������ѡ��ʽ */
 	sig=atoi(getparm("signature"));
	if (getparm("usesignature")[0] != 'Y') sig = 0;
	else if (atoi(getparm("randomsig"))) {
		srand((unsigned) time(NULL));
		i = count_signature();
		if (i) sig = rand() % i + 1;
	}

  	content=getparm("text");

	/* Henry: ������ʱ��½����� */
	if (!loginok) {
		do_login();
		if (strlen(currentuser.userid) == 0 || strcasecmp(currentuser.userid, "guest") == 0)
			http_fatal("��½ʧ�ܣ����Ժ�����");
		loginok = YEA;
	}
	brd=getbcache(board);
	if(brd==0) http_fatal("���������������");
	strcpy(board, brd->filename);
  	for(i=0; i<strlen(title); i++)
		if(title[i]<=27 && title[i]>=-1) title[i]=' ';
   	if(title[0]==0)
      		http_fatal("���±���Ҫ�б���");
      	sprintf(dir, "boards/%s/.DIR", board);
        if(!has_post_perm(&currentuser, board)) {
		if (!user_perm(&currentuser, PERM_WELCOME)) http_fatal("����δ����id���޷��������¡�</ br> ����̳̣�<a href=http://bbs.sysu.edu.cn/teach/teach_http.html>http://bbs.sysu.edu.cn/teach/teach_http.html</a>");
	    	http_fatal("����������Ψ����, ����������Ȩ���ڴ˷�������.");
	}
	if(deny_me(board))
		http_fatal("�ܱ�Ǹ, �㱻������Աֹͣ�˱����postȨ��.");
		
	if(abs(time(0) - *(int*)(u_info->from+36))<6) {
		*(int*)(u_info->from+36)=time(0);
		http_fatal("���η��ļ������, ����Ϣ���������");
	}
	
	if (!oldtitle[0] || strncmp(oldtitle, title, TITLELEN - 2))
		if (!strncmp(title, "Re: ", 4)) {
			title[0]='r';
		}
	*(int*)(u_info->from+36)=time(0);
	strcpy(filename, "tmp/post.XXXXXX");
	fd = mkstemp(filename);
	if (fd == -1) http_fatal("�޷�������ʱ�ļ�");
	if (write(fd, content, strlen(content)) == -1)
		http_fatal("�޷���������");
#ifdef FILTER   
	/* babydragon: �ؼ��ֹ��� */
	if(check_text(title, content) == 0){
		char title_bad[80];
		snprintf(title_bad, sizeof(title_bad), "[�ؼ���][������:%s] %s", board, title);
		post_article("Arbitrate", title_bad, fd, currentuser.userid, currentuser.username, fromhost, -1);
		http_fatal("���������к��в����ʵ�����, �޷�����");
	}
#endif
	r=post_article(board, title, fd, currentuser.userid, currentuser.username, fromhost, sig-1);
	close(fd);
	unlink(filename);
	if (r == -2) http_fatal("��Ч�ĸ�����չ��");
	if(r<=0) http_fatal("�ڲ������޷�����");
	sprintf(buf, "M.%d.A", r);
	brc_initial(board);
	brc_addlist(buf);
	brc_update();

#ifdef ATTACH_UPLOAD
	save_attach(board, r);
#endif

	refile = getparm("refile");
	if (refile) 
		fileinfo = get_file_ent(board, refile);
	/* ԭ���������û��ļĵ��������� */
	if (fileinfo && (fileinfo->flag & FILE_MAIL) && !(brd->flag & ANONY_FLAG)) {
		setboardfile(filepath, board, buf);
		mail_file(filepath, fileinfo->owner, title);
	}
	if(!junkboard(board)) {
        	currentuser.numposts++;
		save_user_data(&currentuser);
		if (fileinfo) {
			write_posts(board, fileinfo->id);
		} else {
			write_posts(board, r);
		}
	}

	if (fileinfo) {
		BBS_SINGAL("/post/reply",
			   "b", board,
			   "f", buf,
			   "f0", fileinfo->filename,
			   NULL);
	} else {
		BBS_SINGAL("/post/newtopic",
			   "b", board,
			   "f", buf,
			   NULL);
	}

	report("posted '%s' on '%s'", title, board);
//	sprintf(buf, "bbsdoc?board=%s", board);
//	redirect(buf);
	printf("<script>setTimeout(\'self.location=\"bbsdoc?board=%s\"\',1);</script>", 
		board);
	http_quit();
	return 0;
}

/* Henry: borrowed from telnet code 2002.12.12 */
int write_posts(char *board, unsigned int id)
{
        struct postlog log;
         
        if (normalboard(board) == NA)
                return YEA;

        strlcpy(log.board, board, BFNAMELEN);
	log.id = id;
        log.date = time(NULL);
        log.number = 1;
        
        if (!if_exist_id(id))
                append_record(".post", &log, sizeof(log));
        append_record(".post2", &log, sizeof(log));

	return YEA;
}

int if_exist_id(unsigned int id) 
{
        static struct mypostlog my_posts;
        char buf1[256];
        int n;
        FILE *fp;
                 
        sethomefile(buf1, currentuser.userid, "my_posts");
        if ((fp = fopen(buf1, "r+")) == NULL) {
                if ((fp = fopen(buf1, "w+")) == NULL)
                        return 0;
        }
        fread(&my_posts, sizeof (my_posts), 1, fp);
        for (n = 0; n < 64; n++)
		if (my_posts.id[n] == id) {		
                        fclose(fp);
                        return 1;
                };
        my_posts.hash_ip = (my_posts.hash_ip + 1) & 63;
	my_posts.id[my_posts.hash_ip] = id;
	fseek(fp, 0, SEEK_SET);
        fwrite(&my_posts, sizeof (my_posts), 1, fp);
        fclose(fp);
        return 0;
}
/*
{
        static struct {
                int hash_ip; 
                char title[64][60];
        } my_posts;
                
        char buf1[STRLEN];
        int n, fd;
                 
        setuserfile(buf1, "my_posts");
        if ((fd = open(buf1, O_RDWR)) == -1) {
                if (read(fd, &my_posts, sizeof(my_posts)) != sizeof(my_posts)) {
                        for (n = 0; n < 64; n++) {
                                if (!strcmp(my_posts.title[n], title)) {
                                        close(fd);
                                        return 1;
                                }
                        }
                }
                lseek(fd, (off_t) 0, SEEK_SET);
        } else {
                if ((fd = creat(buf1, 0644)) == -1)
                        return 0;
        } 
        my_posts.hash_ip = (my_posts.hash_ip + 1) & 63;
        strlcpy(my_posts.title[my_posts.hash_ip], title, sizeof(my_posts.title[my_posts.hash_ip]));
        write(fd, &my_posts, sizeof(my_posts));
        close(fd);
        return 0;
}
*/

int edit_article() {
   	FILE *fp;
   	int type=0, l;
	char buf[512], path[STRLEN], file[STRLEN], board[STRLEN], title[82];
 	struct boardheader *brd;
   	struct fileheader  *x;
   	init_all();
	modify_user_mode(EDIT);
	if(!loginok) http_fatal("�Ҵҹ��Ͳ����޸����£����ȵ�¼");
	strsncpy(board, getparm("board"), 20);
	strsncpy(title, getparm("title"), TITLELEN);
	type=atoi(getparm("type"));
	brd=getbcache(board);
	if(brd==0) http_fatal("�����������");
	strcpy(board, brd->filename);
	strsncpy(file, getparm("file"), 20);
	if(!has_post_perm(&currentuser, board))
		http_fatal("�������������������Ȩ�ڴ���������������");
   	x=get_file_ent(board, file);
	if(strstr(file, "..") || strstr(file, "/")) http_fatal("����Ĳ���");
	if(x==0) http_fatal("����Ĳ���");
/* --------- modify by wood to allow BM to edit the article ---------- */ 
	if (!has_BM_perm(&currentuser, board)
	    && !is_owner(&currentuser, x))
		http_fatal("����Ȩ�޸Ĵ�����");
	/* Henry: ��ֹ�޸���ǰID�����߷�������� */
	if (currentuser.firstlogin > atoi(x->filename + 2))
		http_fatal("����Ȩ�޸Ĵ�����");
	printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
	printf("<html>\n");
	printf("<head>\n");
	printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("<center>%s -- �޸����� [ʹ����: %s]<hr color=green>\n", BBSNAME, currentuser.userid);
	if (type != 0) {
		update_form(board, file, title);
	}
   	printf("<table border=1>\n");
	printf("<tr><td>");
	printf("<tr><td><form method=post action=bbsedit>\n");
	printf("ʹ�ñ��⣺<input type=\"text\" name=\"title\" value=\"%s\" maxlength=40 size=\"44\">  ������:%s<br>\n", nohtml(x->title), board);   /* freestyler:���޸ı��� */

//   	printf("ʹ�ñ���: %s ������: %s<br>\n", nohtml(x->title), board);
   	printf("�������ߣ�%s<br>\n", currentuser.userid);
   	printf("<textarea name=text rows=20 cols=80 wrap=physicle>");
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("�ļ���ʧ");
	l=0;
	while(1) {
		if(fgets(buf, 500, fp)==0) break;
		if(l<4) {
			l++;
			continue;
		}
                if(strcasestr(buf, "--\n")) break;
		if(!strcasestr(buf, "</textarea>")) printf("%s", nohtml(buf));
	}
	fclose(fp);
   	printf("</textarea>\n");
   	printf("<tr><td class=post align=center>\n");
	printf("<input type=hidden name=type value=1>\n");
	printf("<input type=hidden name=board value=%s>\n", board);
	printf("<input type=hidden name=file value=%s>\n", file);
	printf("<input type=submit value=����> \n");
   	printf("<input type=reset value=����></form>\n");
	printf("</table>\n");
	printf("</body>\n");
	printf("</html>\n");
	http_quit();
	return 0;
}

int update_form(char *board, char *file, char *title) {
	FILE *fp;
	struct fileheader *f;
	char *buf=getparm("text"), path[80], tmp[500], texthead[2024], sig[3036], dir[80];

#ifdef FILTER
	        /* babydragon: �ؼ��ֹ��� */
	        if(check_text("", buf) == 0)
	                http_fatal("���������к��в����ʵ�����, �޷�����");
#endif
	sprintf(path, "boards/%s/%s", board, file);
	fp = fopen(path, "r");
	f = get_file_ent(board, file);
	if (fp == NULL || f == NULL) http_fatal("�޷�����");
	my_ansi_filter(title);
	
/* ---------  get the first 4 line of the article --------- */
	fgets(texthead, 500, fp); /* ������ */
	fgets(tmp, 500, fp);  /* read away */
	sprintf(tmp, "��  ��: %s\n", title);
	strcat(texthead, tmp); /* 2 */
	fgets(tmp, 500, fp); 
	strcat(texthead, tmp); /* 3 */
	fgets(tmp, 500, fp); 
	strcat(texthead, tmp); /* 4 */

/* --------  get the signiture --------- */
	while(1) {
		if(fgets(tmp, 500, fp)==0) break;
		if(strcasestr(tmp, "--\n")) {
			strlcpy(sig, tmp, 4);
			while(1) {
				if(fgets(tmp, 500, fp)==0) break;
				strcat(sig, tmp);
				if(strcasestr(tmp, "�� ��Դ:��")) break;
			}
			break;
		}
	} 
	fclose(fp);
	fp = fopen(path, "w");
	if (fp == NULL) return -1;
	fputs(texthead, fp);
	fputs(buf, fp);
	if (buf[strlen(buf) - 1] != '\n')
		fprintf(fp, "\n");	/* Henry: ��ֹ���ǩ������ʾ���� */

	fputs(sig, fp);
	sprintf(tmp, "\x1b[1;36m�� �޸�:��%s � %s �޸ı��ģ�[FROM: %s] \n", 
			anonyboard(board) ? board:currentuser.userid, 
			Cdtime(time(0)), 
			anonyboard(board) ? "������ʹ�ļ�" : fromhost);
	
	fputs(tmp, fp);
	fclose(fp);

	if (title[0] != '\0' && strcmp(title, f->title)) {
		strlcpy(f->title, title, TITLELEN);
		setboardfile(dir, board, ".DIR");
		struct boardheader *bp = getbcache(board);
		safe_substitute_record(dir, f, bp->total, YEA); /* freestyler: update DIR file */
	}

	BBS_SINGAL("/post/updatepost",
	           "f", file,
		   "b", board,
		   NULL);
	BBS_SINGAL("/post/changetitle",
	           "f", file,
		   "b", board,
		   NULL);
	
	sprintf(tmp, "%s %-12s edited %48.48s on %s\n", 
		Cdtime(time(0)), currentuser.userid, f->title, board);
	file_append("wwwlog/trace", tmp);
	http_fatal("<p align='center'>�޸����³ɹ�.<br><a href=bbsdoc?board=%s>[���ر�������]</a>", board);
	return 0;
}

int copy_article() {
	struct fileheader *x;
	char board[BFNAMELEN + 1], file[80], target[BFNAMELEN + 1];
	struct boardheader *bptr;
	init_all();

	strlcpy(board, getparm("board"), sizeof(board));
	bptr = getbcache(board);
	if (bptr == NULL) http_fatal("�����������");
	strlcpy(board, bptr->filename, sizeof(board));
	strlcpy(target, getparm("target"), sizeof(target));
	bptr = getbcache(target);
	if (bptr)
		strlcpy(target, bptr->filename, sizeof(target));
	strsncpy(file, getparm("file"), 30);
	if(!loginok) http_fatal("�Ҵҹ��Ͳ��ܽ��б������");
	if(!has_read_perm(&currentuser, board)) http_fatal("�����������");

	if (!has_post_perm(&currentuser, board))
		http_fatal("��û��Ȩ��ת�أ�");
//	if(!user_perm(&currentuser, PERM_BOARDS|PERM_OBOARDS|PERM_SYSOP))
//		 http_fatal("��û��Ȩ��ת�أ�");
	if (!strcasecmp(target, board))		//Add by Henry: 03.02.15
		http_fatal("ͬһ���滹Ҫת����");

//	if(!check_max_post(board)) http_fatal("�Բ��𣬱����������Ѵﵽ���ޣ����޷�����������ת�����£�");
	x=get_file_ent(board, file);
	if(x==0) http_fatal("������ļ���");
	if(target[0]) {
	        // if(!check_max_post(target)) http_fatal("�Բ��𣬱����������Ѵﵽ���ޣ����޷�����������ת�����£�");
		if(!has_post_perm(&currentuser, target)) http_fatal("��������������ƻ���û���ڸð淢�ĵ�Ȩ��");
		return do_ccc(x, board, target);
	}
	hs_init(10);
	hs_setfile("bbsccc.ptn");
	hs_assign("USER", currentuser.userid);
	hs_assign("TITLE", nohtml(x->title));
	hs_assign("OWNER", x->owner);
	hs_assign("BOARD", board);
	hs_assign("FILE", file);
	hs_end();
	http_quit();
	return 0;
}

int do_ccc(struct fileheader *x, char *board, char *board2) {
	FILE *fp, *fp2;
	char title[512], buf[512], path[STRLEN], path2[STRLEN];
	int fd, i;
	struct boardheader *bh;
	setboardfile(path, board, x->filename);
	fp = fopen(path, "r");
	if (fp == NULL) http_fatal("�ļ������Ѷ�ʧ, �޷�ת��");
	strcpy(path2, "tmp/post.XXXXXX");
	fd = mkstemp(path2);
//	sprintf(path2, "tmp/%d.tmp", getpid());
//	fp2=fopen(path2, "w");
	fp2 = fdopen(fd, "r+");
	for(i=0; i<3; i++)
		if(fgets(buf, 256, fp)==0) break;
	fprintf(fp2, "\033[1;37m�� ��������ת���� \033[32m%s \033[37m������ ��\n", board);
	fprintf(fp2, "�� ԭ����\033[32m %s\033[37m ������ ��\033[m\n\n", x->owner);
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		fprintf(fp2, "%s", buf);
	}
	/* get attach info */
	bh = getbcache(board);
	if (bh && (bh->flag & BRD_ATTACH) && (x->flag & FILE_ATTACHED)) { 
		struct  attacheader ah;
		char	afname[BFNAMELEN];
		strcpy(afname, x->filename);
		afname[0] = 'A';
		if (getattach(board, afname, &ah)) {
			struct  stat st;
			snprintf(buf, sizeof(buf), "attach/%s/%s", board, ah.filename);
			if (lstat(buf, &st) != -1) {
				fprintf(fp2, "\033[m\n����: %s (%d KB) ����:\n",
					ah.origname, (int)st.st_size/1024);
				fprintf(fp2, "\033[4mhttp://%s/attach/%s/%d.%s\033[m\n",
					BBSHOST, board, atoi(ah.filename + 2), ah.filetype );
			}
		}
	}
	fclose(fp);
	fflush(fp2);
	  //Henry: needs to be refreshed before postarticle() in some system.
	if(!strncmp(x->title, "[ת��]", 6)) {
		strcpy(title, x->title);
	} else {
		sprintf(title, "[ת��] %.55s", x->title);
	}
	if (post_article(board2, title, fd, currentuser.userid, 
	   currentuser.username, fromhost, -1) == -1)
		return -1;
	close(fd);
	unlink(path2);
	if(!normalboard(board)) { /* freestyler: ת��д��.post  (ʮ����������) */
		write_posts(board, x->id);
	}

	BBS_SINGAL("/post/cross",
		   "f", x->filename,
		   "b", board2,
		   NULL);

	http_fatal("'%s' ��ת���� %s ��.<br>\n", nohtml(title), board2);
	return 0;
}

int mail_article() {
	struct fileheader *x;
	char board[80], file[80], target[80];
	init_all();
	strsncpy(board, getparm("board"), 30);
	strsncpy(file, getparm("file"), 30);
	strsncpy(target, getparm("target"), 30);
	if(!loginok) http_fatal("�Ҵҹ��Ͳ��ܽ��б������");
	if(!has_read_perm(&currentuser, board)) http_fatal("�����������");
	x=get_file_ent(board, file);
	if(x==0) http_fatal("������ļ���");
	printf("<center>%s -- ת��/�Ƽ������� [ʹ����: %s]<hr color=green>\n", BBSNAME, currentuser.userid);
	if(target[0]) {
		if(!strstr(target, "@")) {
			if(!getuser(target)) http_fatal("�����ʹ�����ʺ�");
			strcpy(target, getuser(target)->userid);
		}
		return do_fwd(x, board, target);
	}
	hs_init(10);
	hs_setfile("bbsfwd.ptn");
	hs_assign("USER", currentuser.userid);
	printf("<table><tr><td>\n");
	printf("���±���: %s<br>\n", nohtml(x->title));
	printf("��������: %s<br>\n", x->owner);
	printf("ԭ������: %s<br>\n", board);
	printf("<form action=bbsfwd method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	printf("������ת�ĸ� <input name=target size=30 maxlength=30 value=%s> (������Է���id��email��ַ). <br>\n",
		currentuser.email);
	printf("<input type=submit value=ȷ��ת��></form>");
	http_quit();
	return 0;
}

int do_fwd(struct fileheader *x, char *board, char *target) {
	char title[STRLEN], path[STRLEN];
	sprintf(path, "boards/%s/%s", board, x->filename);
	if(!file_exist(path)) http_fatal("�ļ������Ѷ�ʧ, �޷�ת��");
	sprintf(title, "[ת��] %s", x->title);
	title[60] = 0;
	if (post_mail(target, title, path, currentuser.userid, 
	   currentuser.username, fromhost, -1, 0) == -1)
		return -1;	
	printf("������ת�ĸ�'%s'<br>\n", nohtml(target));
	printf("[<a href='javascript:history.go(-2)'>����</a>]");
	return 0;
}

/* Henry: ���������ע�������Ƿ�����������֮�� */
int del_article() {
	FILE *fp;
	struct boardheader *brd;
	struct fileheader f;
	struct userec *u;
	char buf[128], dir[80], path[80], board[80], file[80], *id;
	int num=1;
	init_all();
	if(!loginok) http_fatal("���ȵ�¼");
	id=currentuser.userid;
	strsncpy(board, getparm("board"), 60);
	strsncpy(file, getparm("file"), 20);
	brd=getbcache(board);
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2)) http_fatal("����Ĳ���");
	if(strstr(file, "..")) http_fatal("����Ĳ���");
	if(brd==0) http_fatal("�������");
	if(!has_post_perm(&currentuser, board)) http_fatal("�����������");
	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("����Ĳ���");
	while(1) {
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0) break;
		if(!strcmp(f.filename, file)) {
        		if(!has_BM_perm(&currentuser, board) && !is_owner(&currentuser, &f))
                		http_fatal("����Ȩɾ������");

			if (delete_record(dir, sizeof(struct fileheader), num) < 0)
				http_fatal("ɾ��ʧ��");
			sprintf(buf, "%s %-12s delete %-48.48s on %s\n", 
				Cdtime(time(0)), id, f.title, board);
			file_append("wwwlog/trace", buf);

			if (!((brd->flag & ANONY_FLAG) && is_owner(&currentuser, &f))) {
				snprintf(buf, sizeof(buf), "%-32.32s - %s", f.title, currentuser.userid);
				strlcpy(f.title, buf, TITLELEN);
			}
			strlcpy(f.filename, file, sizeof(f.filename));

			/* Henry: clear fileheader's m & g bits */
			f.flag = 0;

			if (is_owner(&currentuser, &f)) {
				setboardfile(dir, board, ".JUNK");
			} else {
				setboardfile(dir, board, ".DELETED");
			}
			append_record(dir, &f, sizeof(f));

			printf("ɾ���ɹ�.<br><a href='bbsdoc?board=%s'>���ر�������</a>", board);

			BBS_SINGAL("/post/del",
				   "b", board,
				   "f", file,
				   NULL);
			BBS_SINGAL("/post/cancelpost",
				   "b", board,
				   "f", file,
				   NULL);

			u=getuser(f.owner);
			if(!junkboard(board) && u) {
				if(u->numposts>0) u->numposts--;
				save_user_data(u);
			}
			update_lastpost(board);
			http_term();
		}
		num++;
	}
	fclose(fp);
	printf("�ļ�������, ɾ��ʧ��.<br>\n");
	printf("<a href='bbsdoc?board=%s'>���ر�������</a>", board);
	http_quit();
	return 0;
}

int is_owner(struct userec *user, struct fileheader *fileinfo) {
        if ((strcmp(fileinfo->owner, user->userid)) &&
             (strcmp(fileinfo->realowner, user->userid)))
                return NA;
    
        return (fileinfo->filetime < user->firstlogin) ? NA : YEA;
}
