#include "webbs.h"

struct stat *
f_stat (file)
char *file;
{
        static struct stat buf;
        
        memset(&buf, 0, sizeof(buf));
        if(stat(file, &buf)==-1) memset(&buf, 0, sizeof(buf));
        return &buf;
}

int fprintf2(fp, s) 
FILE *fp;
char *s;
{
        int i, tail=0, sum=0;
        if(s[0]==':' && s[1]==' ' && strlen(s)>79) {
                sprintf(s+76, "..\n");
	        return fprintf(fp, "%s", s); 
        }
        for(i=0; s[i]; i++) {
		/* Henry: 把tab转换成空格 */
		if (s[i] == '\t') {
			do {
				sum++;
				fprintf(fp, " ");
			} while (sum % 8 != 0 && sum < 78);
			tail = 0;
			if (sum == 78) {
				fprintf(fp, "\n");
				sum = 0;
			}
			continue;
		}
                fprintf(fp, "%c", s[i]);
                sum++;
		/* Henry: 汉字高位字节为1 */
                if(tail) {
                        tail=0;
                } else if (s[i] & 0x80) {
                        tail=s[i]; 
                }
                if(sum>=78 && tail==0) {
                        fprintf(fp, "\n");
                        sum=0;
                }
        }
	return sum;
}

int 
post_inform (board, title, filename)
char *board;
char *title;
char *filename;
{
	struct fileheader fh;
	char fname[STRLEN];
	char buf[256];
	FILE *fp1, *fp2;
	int i;
	time_t now;

	fp1 = fopen(filename, "r");
	if (fp1 == NULL) return -1;

	i = getnewfilename(board);
	if (i == -1) return -1;
	fh.id = i;
	sprintf(fh.filename, "M.%d.A", i);
	setboardfile(fname, board, fh.filename);
	fp2 = fopen(fname, "w");
	if (fp2 == NULL) return -1;

	strcpy(fh.owner, BBSID);
	strlcpy(fh.title, title, TITLELEN);
	fh.flag = FILE_MARKED | FILE_NOREPLY;
	fh.size = file_size(filename);
	now = time(NULL);
	fh.filetime = now;
	setboardfile(fname, board, ".DIR");
	append_record(fname, &fh, sizeof(fh));

        sprintf(buf, "\033[1;41;33m发信人: %s (自动发信系统), 信区: %s",
		BBSID, board);
	fprintf(fp2, "%s%*s\033[m\n", buf, (int)(89 - strlen(buf)), " ");
        fprintf(fp2, "标  题: %s\n", title);
        fprintf(fp2, "发信站: %s (%24.24s)\n", BBSNAME, ctime(&now));
	fprintf(fp2, "\n");
	file_copy(fp1, fp2);

	fprintf(fp2, "\n\n\n--\n");
        fprintf(fp2, "\033[m\033[1;%dm※ 来源:．%s %s [FROM: %.20s]\33[m\n", 31+rand()%7,
                BBSNAME, BBSHOST, BBSHOST);
        fclose(fp2);
	fclose(fp1);
	return 0;	
}

int 
post_article (board, title, fd, id, nickname, ip, sig)
char *board;
char *title;
int fd;
char *id;
char *nickname;
char *ip;
int sig;
{
        FILE *fp, *fp2;
        char buf[1024], anonymous[2], exchange[2], noreply[2], dir[80];
        struct fileheader header;
        int t, noname=0;
	int articleid;

        strlcpy(anonymous, getparm("anonymous"), sizeof(anonymous));
        strlcpy(exchange, getparm("exchange"), sizeof(exchange));  /* Henry: 转信 */
	strlcpy(noreply, getparm("noreply"), sizeof(noreply));	/* 不可回复 */
	strlcpy(buf, getparm("articleid"), sizeof(buf));
	articleid = atoi(buf);
        if (anonymous[0] == 'Y' && anonyboard(board)) noname = 1;

	setboardfile(dir, board, ".DIR");
	if (!file_exist(dir)) utime(dir, NULL);

	/* Henry: 第一篇文章的article id等于文件名中间那个部分，
	re文和它的祖先的article id一样。*/

        memset(&header, 0, sizeof(header)); 
	t = getnewfilename(board);
	if (t == -1) return -1;
	if (articleid) {
		header.id = articleid;
	} else {
		header.id = t;
	}
        sprintf(header.filename, "M.%d.A", t);
	header.filetime = t;

	/* 置不可回复标记 */
	header.flag = (noreply[0] == 'Y') ? FILE_NOREPLY : 0;

	strlcpy(header.realowner, id, sizeof(header.realowner));
	strlcpy(header.owner, (noname) ? board : id, sizeof(header.owner));
        strlcpy(header.title, title, sizeof(header.title));

#ifdef ATTACH_UPLOAD
	/* Henry: set attachment flag */
	struct boardheader *bptr;
	char *uploadfname, *ext;
	char fname[80];

	uploadfname = getparm("UPLOADFILE");
	if (strlen(uploadfname) > 0) {
		bptr = getbcache(board);
		if (bptr != NULL && (bptr->flag & BRD_ATTACH))
			header.flag |= FILE_ATTACHED;
		ext = strrchr(uploadfname, '.');
		if (ext == NULL) return -2;
		strtolower(buf, ext + 1);
		snprintf(fname, sizeof(fname), "attach/%s/upload.ext", board);
		if (!seek_in_file(fname, buf)) return -2;
		header.attachid = t;
	}
#endif

	setboardfile(buf, board, header.filename);
        fp=fopen(buf, "w");
	if (fp == NULL) http_fatal("发贴错误1，请于系统维护联系");
	fp2=fdopen(fd, "r+");
	if (fp2 == NULL) http_fatal("发贴错误2，请于系统维护联系");
	fseek(fp2, 0, SEEK_SET);
        fprintf(fp, "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%24.24s)\n\n",
                noname ? board : id,
                noname ? "我是匿名天使" : nickname, board, title, BBSNAME, Ctime(time(0)));
                while(1) {
	                if(fgets(buf, 1000, fp2)<=0) break;
	                fprintf2(fp, buf); 
                }
                fclose(fp2);

        fprintf(fp, "\n--\n");
        if (!noname) sig_append(fp, id, sig);
        fprintf(fp, "\033[m\n\033[1;%dm※ 来源:．%s http://%s [FROM: %.20s]\33[m\n", 31+rand()%7,
                BBSNAME, BBSHOST, noname ? "匿名天使的家" : ip);
        fclose(fp);
	setboardfile(buf, board, ".DIR");
		
	append_record(buf, &header, sizeof(header));


        if (exchange[0] == 'Y') {     /* Henry: 转信 */   
                sprintf(buf, "%s/innd/out.bntp", BBSHOME);
                if ((fp = fopen (buf, "a")) != NULL) {
                        fprintf (fp, "%s\t%s\t%s\t%s\t%s\n", board, header.filename,
                                header.owner, id, header.title);
                        fclose (fp);
                }
        }
        update_lastpost(board);
        return t;
}

int sig_append(FILE *fp, char *id, int sig) {
        FILE *sig_fp;
        char path[256], buf[256];
        int i;
        struct userec *x;
        if(sig<0 || sig>99) return -1;
        x=getuser(id);
        if(x==0) return -1;
        sprintf(path, "home/%c/%s/signatures", toupper((int)id[0]), id);
        sig_fp = fopen(path, "r");
        if (sig_fp == NULL) return -1;
	for (i = 0; i < sig * 6; i++)
		if (fgets(buf, sizeof(buf), sig_fp) == 0) {
			fclose(sig_fp);
			return -1;
		}
	for (i = 0; i < 6; i++) {
		if (fgets(buf, sizeof(buf), sig_fp) == 0) break;
		fprintf(fp, "%s", buf);
	}
	fclose(sig_fp);
	return 0;
}

char *
eff_size (file)
char *file; {
        FILE *fp;
        static char buf[256];
        int i, size, size2=0;
        size=file_size(file);
        if(size>3000|| size==0) goto E; 
        size=0;
        fp=fopen(file, "r");
        if(fp==0) return "-";
        for(i=0; i<3; i++)
                if(fgets(buf, 255, fp)==0) break;
        while(1) {
                if(fgets(buf, 255, fp)==0) break;
                if(!strcmp(buf, "--\n")) break;
                if(!strncmp(buf, ": ", 2)) continue;
                if(!strncmp(buf, "【 在 ", 4)) continue;
                if(strstr(buf, "※ 来源:．")) continue;
//              if(strstr(buf, "※ 修改:．")) continue;
                for(i=0; buf[i]; i++) if(buf[i]<0) size2++;
                size+=strlen(trim(buf));
        }
        fclose(fp);
E:
        if(size<2048) {
                sprintf(buf, "(<font style='font-size:12px; color:#008080'>%d字</font>)", size-size2/2);
        } else {
                sprintf(buf, "(<font style='font-size:12px;color:#f00000'>%d.%d千字</font>)", size/1000, (size/100)%10);
        }
        return buf;
}

/* copy from telnet source code */
int getfilename(char *basedir, char *filename, int flag, int *id)
{
        char fname[PATH_MAX + 1], ident = 'A';
        int fd = 0, count = 0;
        time_t now = time(NULL);

        while (1) {
                if (count++ > MAX_POSTRETRY)
                        return -1;

                snprintf(fname, sizeof(fname), "%s/M.%d.%c", basedir, (int)now, ident);
                if (flag & GFN_LINK) {
                        if(link(filename, fname) == 0) {
                                unlink(filename);
                                break;
                        }
                }
		else {
                        if ((fd = open(fname, O_CREAT | O_EXCL | O_WRONLY, 0644)) != -1) {
                                if (!(flag & GFN_NOCLOSE)) {
                                        close(fd);
                                        fd = 0;
                                }
                                break;
                        }

                        if (errno == EEXIST) {
				// monster: 仅当文件存在时才重试
                                if (!(flag & GFN_SAMETIME) || ident == 'Z') {
                                        ident = 'A';
                                        ++now;
                                }
				else {
                                        ++ident;
                                }
                                continue;
                        }
                }
                return -1;
        }
	
        if ((flag & GFN_UPDATEID) && id != NULL)
                *id = now;
	
        strlcpy(filename, fname, sizeof(fname));
        return fd;
}

int 
getnewfilename (board)
char *board; {
	int i, now, fd;
	char filename[16], buf[STRLEN];
	now = time(0);
	setboardfile(buf, board, ".DIR");
	fd = filelock(buf, YEA);
	for (i = 0; i<100; i++) {
		snprintf(filename, sizeof(filename), "M.%d.A", now + i);
		setboardfile(buf, board, filename);
		if (!file_exist(buf)) {
			/* Henry: 防止其他并行进程覆盖掉该文件 */
			utime(buf, NULL);
			break;
		}
	}
	fileunlock(fd);
	if (i > 99) return -1;
	return now + i;
}

int file_copy(FILE *fp1, FILE *fp2) {
	char buf[1024];
	if (fp1 == NULL || fp2 == NULL) return -1;
        while(1) {
                if(fgets(buf, 1000, fp1)<=0) break;
                fprintf(fp2, "%s", buf);
        }
	return 0;
}

int disp_file(char *filename) {
	char filebuf[1024];
	FILE *fp;
	char *id, *s, *ptr, userid[IDLEN + 1];

	if (strstr(filename, "..")) return -1;
	fp = fopen(filename, "r");
	if (fp == NULL) return -1;

	userid[0] = '\0';
	set_color(37);
	while (1) {
		if (fgets(filebuf, sizeof(filebuf), fp) == 0) break;
		if (!strncmp(filebuf, "发信人: ", 8)) {
			ptr = strdup(filebuf);
			id = strtok(ptr + 8, " ");
			strlcpy(userid, id?id:"", sizeof(userid));
			s = strtok(0, "");
			if (id == 0) id = " ";
			if (s == 0) s = "\n";
			if (getuser(userid)) {
                                printf("发信人: %s%s", userid_str(userid), s);
				free(ptr);
				continue;
                        }
                        free(ptr);
                }
                if(!strncmp(filebuf, ": ", 2))
			printf("<font color=808080>");
		else
			printf("<font class=%d>", get_color());
                hhprintf("%s", void1(filebuf));
                printf("</font>");
	}
	fclose(fp);

#ifdef ATTACH_UPLOAD
	char attfname[STRLEN], *board;

	strtok(filename, "/");
	board = strtok(NULL, "/");
	strlcpy(attfname, strtok(NULL, ""), sizeof(attfname));
	attfname[0] = 'A';
	show_attach_link(board, attfname);
#endif
	return 0;
}

int is_picture(struct attacheader *attptr) {
	return (!strcasecmp(attptr->filetype, "jpg") || 
		!strcasecmp(attptr->filetype, "bmp") ||
		!strcasecmp(attptr->filetype, "gif") || 
		!strcasecmp(attptr->filetype, "jpeg") ||
		!strcasecmp(attptr->filetype, "png")
	);
}
