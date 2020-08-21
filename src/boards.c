#include "webbs.h"

int 
isOutgoingBoard (board)
char *board;       /* Add by Henry */
{  
        struct boardheader *bp;
        bp=getbcache(board);
        return (bp->flag & OUT_FLAG);
}

int 
junkboard (board)
char *board; {
        struct boardheader *bp;
        bp=getbcache(board);
        return (bp->flag & JUNK_FLAG);
}


/* freestyler: borrowed from telnet src, 2008.7.23*/
int 
normalboard(char* bname)
{
	struct boardheader *bp;

	if ((bp = getbcache(bname)) == NULL)
		return NA;
	if (bp->flag & (ANONY_FLAG | JUNK_FLAG | BRD_RESTRICT | BRD_NOPOSTVOTE))
		return NA;
	
	return (bp->level == 0) ? YEA : NA;
}

int 
anonyboard (board)
char *board; {
        struct boardheader *bp;
        bp=getbcache(board);
        return (bp->flag & ANONY_FLAG);
}

char *getsecname(board)
char *board;
{
	int secnum = getsecnum(board);
	if (secnum == -1) return "";
	return secname[secnum][0];
}

int 
getsecnum (board)
char *board; {
	struct boardheader *data; 
	int i;

        if (!board) return -1;
	data=getbcache(board);
        if (data) {
		for (i=0; i<SECNUM; i++)
			if (strchr(seccode[i], data->title[0]))
				return i;
	}
	return -1;
}

char *
sec (c)
char c; {
        int i;
        for(i=0; i<SECNUM; i++) {
                if(strchr(seccode[i], c)) return secname[i][0];
        }
        return NULL;
}

struct fileheader *
get_file_ent (board, file)
char *board;
char *file; {
	int fd;
        char dir[80];
        static struct fileheader x;
        int num=0;
	setboardfile(dir, board, ".DIR");
	fd = open(dir, O_RDONLY);
	if (fd == -1) return NULL;
        while(1) {
		if (read(fd, &x, sizeof(x)) <= 0) break;
                if(!strcmp(x.filename, file)) {
			close(fd);
                        return &x;
                }
                num++;
        }
	close(fd);
        return NULL;
}

char *
flag_str (access)
int access; {
        static char buf[80];
        char *flag2="";
        strcpy(buf, "  ");
        if(access & FILE_DIGEST) flag2="G";
        if(access & FILE_MARKED) flag2="M";
        if((access & FILE_MARKED) && (access & FILE_DIGEST)) flag2="B";
	if (access & FILE_ATTACHED) flag2 = "@";
        sprintf(buf, "%s", flag2);
        return buf;
}
        
char *
flag_str2 (access, has_read)
int access;
int has_read; {
        static char buf[80];
        strcpy(buf, "   "); 
        if(loginok) strcpy(buf, "N  ");
        if(access & FILE_DIGEST) buf[0]='G';
        if(access & FILE_MARKED) buf[0]='M';
        if((access & FILE_MARKED) && (access & FILE_DIGEST)) buf[0]='B';
        if (has_read) buf[0] = tolower((int)buf[0]);
	if (access & FILE_ATTACHED) buf[0] = '@';
        if(buf[0]=='n') buf[0]=' ';
	buf[1] = '\0';
        return buf;
}

int 
cmpboard (b1, b2)
const void *b1;
const void *b2;
{
        return strcasecmp(((struct boardheader *)b1)->filename, 
		((struct boardheader *)b2)->filename);
}

int
cmpboardname(const void *b1, const void *b2)
{
	return strcasecmp((char *) b1,
		((struct boardheader *)b2)->filename);
}

int 
filenum (board)
char *board; {
        char file[256];
        sprintf(file, "boards/%s/.DIR", board);
        return file_size(file)/sizeof(struct fileheader);
}

int 
board_read (board)
char *board; {
        char fname[256];
	int fd;
        struct fileheader x;
        int total;
        if(!loginok) return 1;
        memset(&x, 0, sizeof(x));
	setboardfile(fname, board, ".DIR");
        total=file_size(fname)/sizeof(struct fileheader);
        if(total<=0) return 1;
	fd = open(fname, O_RDONLY);
        lseek(fd, (total-1)*sizeof(struct fileheader), SEEK_SET);
        read(fd, &x, sizeof(x)); 
	close(fd);
	brc_initial(board);
        return !brc_unread(x.filename);;
}

int show_activeboard() {
	FILE *fp;
	int fd, num, total, i;
	struct fileheader fh;
	char fname[STRLEN], buf[256], *ptr;

	init_all();
	setboardfile(fname, "activeboard", ".DIGEST");
	fd = open(fname, O_RDONLY);
	if (fd == -1) return -1;
	total = file_size(fname) / sizeof(struct fileheader);
	if (total <= 0) return -1;
	num = atoi(getparm("num"));
	if (num >= total) num = num % total;
	printf("<html>");
	printf("<head>"
		"<link href=templates/global.css rel=stylesheet type=text/css />"
		"<style>table{"
		"font-family:'verdana','arial';"
		"font-size:12px;"
		"}</style></head>");
        printf("<meta http-equiv='refresh' content='10; url=bbsactive?num=%d'>\n", num+1);
	lseek(fd, num * sizeof(struct fileheader), SEEK_SET);
	if (read(fd, &fh, sizeof(fh)) <= 0) return -1;
	close(fd);
	setboardfile(fname, "activeboard", fh.filename);
	fp = fopen(fname, "r");
	if (fp == NULL) return -1;
	printf("<center><table><tr><pre><font style='font-size:12px;'>");
	for (i = 0; i < 4; i ++) fgets(buf, sizeof(buf), fp);
	for (i = 0; i < MAXMOVIE; i ++) {
		fgets(buf, sizeof(buf), fp);
		if (feof(fp)) return 0;
		if (!strncmp(buf, "--", 2)) break;
		ptr = strtok(buf, "\r\n");
		hhprintf("%s\n", void1(buf));
	}
 	fclose(fp);
	printf("</font></pre></tr></table>");
	printf("</html>");
	http_quit();
	return 0;
}

int
show_section_title() {
	init_all();
	section_title();
	http_quit();
	return 0;
}

int
show_main_frame(){
	char *target;
	
	init_all();
	target = getparm("target");
	
	printf("<html>"
		"<head>"
		"	<meta http-equiv='Content-Type' content='text/html; charset=gb2312'>"
		"	<title>" BBSNAME "电子公告板系统（BBS）</title>"
		"</head>"
		"<frameset rows='*' cols='175,*' frameborder='NO' border='0' framespacing='0'>"
		"	<frame src='bbsleft' name='f2' scrolling='auto' noresize id='f2'>"
		"	<frameset rows='*,24' frameborder='NO' border='0' framespacing='0'>"
		"		<frame src='%s' name='f3' id='f3'>"
		"   		<frame src='bbsfoot' name='f4' frameborder='no' scrolling='NO' noresize id='f4'>"
		"	</frameset>"
		"</frameset>"
		"<noframes><body></body></noframes>"
		"</html>", target);
	http_quit();
	return 0;
}

int
show_section_frame() {
	printf("<frameset frameborder=0 border=0 rows=\"80, %d, *\">\n", MOVIE_HEIGHT);
	printf("<frame scrolling=no marginwidth=4 marginheight=0 src=\"bbssectitle\">");
	printf("<frame scrolling=no marginwidth=4 marginheight=0 src=\"bbsactive\">\n");
	printf("<frame marginwidth=4 marginheight=0 src=\"bbsallsec\">\n");
	printf("</frameset>\n");
	return 0;
}

int 
show_all_section() 
{
   	int i, j, groupflag;
	struct boardheader *bptr = 0;
	
   	init_all();
	modify_user_mode(SELECT);
	
	printf("<html>"
		"<head>"
		"<meta http-equiv=Content-Type content=text/html; charset=gb2312 />"
		"<title>%s</title>"
		"<link href=templates/global.css rel=stylesheet type=text/css />"
		"</head>"
		"<body>"
	       // insert html here
		"<form name=form1 method=post action=>"
		"<div id=head>"
		"<div id=index_search>"
		"</div>"
		"</div>"
		"<table id=container border=0 cellspacing=0 cellpadding=0>"
		"<tr>"	   
	   	"<td id=center>"
		"<table border=0 cellspacing=0 cellpadding=0 id=boards>"
		"<tr>" 
		"<td colspan=5 class=tb_head>"
		"<img src=images/table_ul02.gif style=float:left class=tb_ul /><div class=title>讨论区地图</div></td>"
		"<td width=19 align=right valign=top class=tb_r><img src=images/table_ur.gif /></td>"
		"</tr>"
		"<tr>"
		"<td><img src=images/table-left2.gif /></td>"
		"<td width=40 class='td2'>区号</td>"
		"<td width=72 class='td2'>类别</td>"
		"<td width=106 class='td2'>描述</td>"
		"<td class=td2>版面</td>"
		"<td><img src=images/table_right2.gif /></td>"
		"</tr>"
		, BBSNAME);
	for(i=0; i<SECNUM; i++) {
		printf("<tr>"
			"<td class=tb_l></td>");
		
		printf("<td class='td3 border'>%d</td>", i);
		printf("<td class='td3 border'><a href=bbsboa?board=%d>%s</td>", i, secname[i][0]);
		printf("<td class='td3 border'><a href=bbsboa?board=%d>%s</a></td>", i, secname[i][1]);
		printf("<td class='td3 border boards'>");
		
		groupflag = 0;
		for(j = 0; j<MAXBOARD; j++)
		{
			if (shm_bcache->bcache[j].filename[0]<=32 || shm_bcache->bcache[j].filename[0]>'z') continue;
			if (!has_read_perm(&currentuser, shm_bcache->bcache[j].filename)) continue;
			if (!groupflag) {
				if (!strchr(seccode[i], shm_bcache->bcache[j].title[0])) continue;
				if (shm_bcache->bcache[j].parent != 0) continue;
			}
			if (groupflag) {
				if (shm_bcache->bcache[j].parent == 0) continue;
				if (&(shm_bcache->bcache[shm_bcache->bcache[j].parent - 1]) != bptr) continue;
			}
			
			char path[256], buf[80], buf1[80], buf2[80];
			int def_mode = 0;
			FILE *fp;
			setuserfile(path, ".mywww");
			fp=fopen(path, "r");
			if(fp) {
				while(1) {
					if(fgets(buf, sizeof(buf), fp)==0) break;
					if(sscanf(buf, "%80s %80s", buf1, buf2)!=2) continue;
					//if(!strcmp(buf1, "t_lines")) t_lines=atoi(buf2);
					//if(!strcmp(buf1, "link_mode")) link_mode=atoi(buf2);
					if(!strcmp(buf1, "def_mode")) def_mode=atoi(buf2);
				}
				fclose(fp);
			}
			if(def_mode == 1)
				printf("<a href=bbstdoc?board=%s>%s</a>",
					shm_bcache->bcache[j].filename, shm_bcache->bcache[j].title + 11);
			else
				printf("<a href=bbsdoc?board=%s>%s</a>", 
					shm_bcache->bcache[j].filename, shm_bcache->bcache[j].title + 11);
		}
		printf("</td>"
			"<td class='tb_r'>&nbsp;</td>"
			"</tr>\n");
	}
	printf("<tr class=tb_bottom>"
		"<td colspan=5 valign=bottom><img src='images/table_bl.gif' /></td>"
		"<td align=right class=tb_bottom><img src='images/table_br.gif' /></td>"
		"</tr>"
		"</table>"
		"</td>"
		"</tr>"
		"</table>"
		"</form>"
		"<script src='http://www.google-analytics.com/urchin.js' type='text/javascript'></script>"
		"<script type='text/javascript'>"
		"_uacct = 'UA-1161962-1';"
		"urchinTracker();"
		"</script>"
		"</body>"
		"</html>");
	http_quit();
	return 0;
}

int show_section() {
	int i, total, sec1;
	char buf1[100], *ptr, *board;
	char buf[10][255];
	char secnum[10];
	struct boardheader data[MAXBOARD], *x, *bptr = 0;
	int groupflag;	/* Henry: 标示是否为二级版 */

	init_all();
	modify_user_mode(READNEW);

	board = getparm("board");
	if (board[0] == '\0') http_fatal("无效的讨论区");
	if (board[0] >= '0' && board[0] <= '9') {
		groupflag = 0;
		sec1 = atoi(board);
		if(sec1<0 || sec1>=SECNUM) http_fatal("无效的讨论区");
	} else {
		groupflag = 1;
		bptr = getbcache(board);
		if (bptr == NULL) http_fatal("无效的讨论区");
		sec1 = getsecnum(board);
	}

	total = 0;
	for(i = 0; i < MAXBOARD; i++) {
		x = &(shm_bcache->bcache[i]);
		if (x->filename[0]<=32 || x->filename[0]>'z') continue;
		if (!has_read_perm(&currentuser, x->filename)) continue;
		if (!groupflag) {
			if (!strchr(seccode[sec1], x->title[0])) continue;
			if (x->parent != 0) continue;
		}
		if (groupflag) {
			if (x->parent == 0) continue;
			if (&(shm_bcache->bcache[x->parent - 1]) != bptr) continue;
		}
		memcpy(&data[total], x, sizeof(struct boardheader));
		total++;
	}

	if (total == 0) http_fatal("本版没有文章");
	qsort(data, total, sizeof(struct boardheader), cmpboard);

	hs_init(20);
	hs_setfile("bbsboa.ptn");
	hs_assign("SECNAME", secname[sec1][0]);
	sprintf(secnum, "%d", sec1);
	hs_assign("SECNUM", secnum);
	
	hs_setloop("mainform");
	for(i=0; i<total; i++) {
		hs_assign("BOARD", data[i].filename);
		hs_assign("BOARDNAME", data[i].title + 8);
		if (data[i].flag & BRD_READONLY) {
			hs_assign("TYPE", "[<font color=red>只读</font>]");
		} else {
			*(data[i].title + 7) = '\0';
			hs_assign("TYPE", data[i].title + 1);
		}
		if (data[i].flag & BRD_GROUP) {
			hs_assign("READFLAG", "＋");
			hs_assign("VOTEFLAG", "");
			hs_assign("BM", "[目录]");
			hs_assign("METHOD", "bbsboa");
			hs_assign("POSTNUM", "-");
			hs_assign("DATE", "-");
			hs_doloop("mainform");
			continue;
		}
		hs_assign("READFLAG", board_read(data[i].filename) ? "◇" : "<font class='new'>◆</font>");
		char path[256], t_buf[80], t_buf1[80], t_buf2[80];
		int def_mode = 0;
		FILE *fp;
		setuserfile(path, ".mywww");
		fp=fopen(path, "r");
		if(fp) {
			while(1) {
				if(fgets(t_buf, sizeof(t_buf), fp)==0) break;
				if(sscanf(t_buf, "%80s %80s", t_buf1, t_buf2)!=2) continue;
				if(!strcmp(t_buf1, "def_mode")) def_mode=atoi(t_buf2);
			}
			fclose(fp);
		}
		if(def_mode == 1)
			hs_assign("METHOD", "bbstdoc");
		else
			hs_assign("METHOD", "bbsdoc");
		hs_assign("VOTEFLAG", (data[i].flag & VOTE_FLAG)?
			"<font color='#ff0000'>V</font>":"");
		ptr=strtok(data[i].BM, " ,;");
		hs_assign("BM", (ptr)?userid_str(ptr):"诚征版主中");
		sprintf(buf[5], "%d", filenum(data[i].filename));
		hs_assign("POSTNUM", buf[5]);
		setboardfile(buf1, data[i].filename, ".DIR");
		sprintf(buf[6], "%12.12s", Cdtime(file_time(buf1)));
		hs_assign("DATE", buf[6]);
		sprintf(buf[7], "%d", sec1);
		hs_assign("SECNUM", buf[7]);
		hs_doloop("mainform");
	}
	hs_end();
	http_quit();
	return 0;
}

int show_good_boards() {
	FILE *fp;
	char buf[4][256], board[80];
	struct boardheader *x1;
	
	init_all();
	//modify_user_mode(READING);
	if( (fp = fopen("etc/sysgoodbrd", "r")) == NULL)
		http_fatal("can't read file");
	hs_init(2);
	hs_setfile("bbsgood.ptn");
	hs_setloop("mainform");
	while(feof(fp) == 0) {
		if(fgets(board, sizeof(board), fp) == NULL)
			break;
		board[strlen(board) - 1] = '\0';
		x1 = getbcache(board);
		if(x1==0)
			continue;
		hs_assign("BOARD", board);
		strcpy(buf[1], x1->title + 11);
		hs_assign("BOARDNAME", buf[1]);
		hs_doloop("mainform");
	}
	hs_end();
	http_quit();
	return 0;
}

int show_board() {
	int my_t_lines, start, total;
	struct fileheader x;
	struct boardheader *x1;
	char board[BFNAMELEN + 1], filename[80], dir[80], *ptr;
  char title_buf[TITLELEN * 6 + 70];
	int fd;
	int i;

	init_all();
	modify_user_mode(READING);
	strsncpy(board, getparm("board"), BFNAMELEN);
	x1=getbcache(board);
	if(x1==0) http_fatal("错误的讨论区");
	strlcpy(board, x1->filename, sizeof(board));
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if(!has_enter_perm(&currentuser, board)&&loginok) http_fatal("对不起，您目前被停止进入本板的权力!");

	show_note(board);
	setboardfile(dir, board, ".DIR");
	fd = open(dir, O_RDONLY);
	if (fd == -1) {
		sprintf(filename, "bbspst?board=%s", board);
		redirect(filename);
		http_term();
	}
        total=file_size(dir)/sizeof(struct fileheader);
	start=atoi(getparm("start"));
	my_t_lines=atoi(getparm("my_t_lines"));
	if(my_t_lines<10 || my_t_lines>40) my_t_lines=20;
        if (start == 0 || start > total - my_t_lines + 1)
		start = total - my_t_lines + 1;
	if (start < 0) start = 1;
	brc_initial(board);
	if (total == 0) {
		sprintf(filename, "bbspst?board=%s", board);
		redirect(filename);
		http_term();
	}

	hs_init(25);
	hs_setfile("bbsdoc.ptn");
	hs_assign("BOARD", board);
	sprintf(hs_genbuf[0], "%d", getsecnum(board));
	hs_assign("SECNUM", hs_genbuf[0]);
	hs_assign("SECNAME", getsecname(board));
	hs_assign("BOARDNAME", x1->title + 11);
	hs_assign("BMLIST", x1->BM[0] ? userid_str(x1->BM) : "诚征版主中");
	if (loginok) hs_assign("LOGINOK", "");
	if (has_BM_perm(&currentuser, board)) hs_assign("HAS_BM_PERM", "");
	if (x1->flag & VOTE_FLAG)
		hs_assign("HASVOTE", "");
	hs_setloop("row");
	lseek(fd, (start - 1)*sizeof(struct fileheader), SEEK_SET);
	for(i=0; i<my_t_lines; i++) {
		if (read(fd, &x, sizeof(x))<=0) break;
		sprintf(filename, "boards/%s/%s", board, x.filename);
		sprintf(hs_genbuf[0], "%d", start+i);
		hs_assign("INDEX", hs_genbuf[0]);
		
		ptr=flag_str2(x.flag, !brc_unread(x.filename));
		if (x.flag & FILE_NOREPLY) {
			if(ptr[0]=='N')
				ptr = "<u>X</u>";
			else
				ptr = "<u>x</u>";
		}
		if (ptr[0] != ' '){
			if(brc_unread(x.filename)){
				sprintf(hs_genbuf[1], "<font class='new'>%s</font>", ptr);
				hs_assign("FLAG", hs_genbuf[1]);
			}
			else
				hs_assign("FLAG", ptr);
		}
		else
			hs_assign("FLAG", "&nbsp;");
		
		sprintf(title_buf, "<a href=bbscon?board=%s&file=%s>%s%s</a>",
        		board, x.filename,
			strncmp(x.title, "Re: ", 4) ? "○ " : "",
        		void1(nohtml(x.title)));
		hs_assign("TITLE", title_buf);
		hs_assign("WORDCOUNT", eff_size(filename));
		hs_assign("AUTHOR", x.owner);
		sprintf(hs_genbuf[3], "%12.12s", Cdtime(atoi(x.filename+2)));
		hs_assign("DATE", hs_genbuf[3]);
		hs_doloop("row");
	}
	close(fd);
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
		sprintf(hs_genbuf[4], "<a href=bbsdoc?board=%s&start=1>首页</a> <a href=bbsdoc?board=%s&start=%d>上一页</a>",
			board, board, start - my_t_lines);
	hs_assign("PGBUTTONUP", hs_genbuf[4]);
	if (start >= total - my_t_lines + 1)
		strcpy(hs_genbuf[5], "<font color=ff0000>下一页 末页</font>");
	else
		sprintf(hs_genbuf[5], "<a href=bbsdoc?board=%s&start=%d>下一页</a> <a href=bbsdoc?board=%s&start=%d>末页</a>",
			board, start + my_t_lines, board, total - my_t_lines + 1);
	hs_assign("PGBUTTONDN", hs_genbuf[5]);
	sprintf(hs_genbuf[6], "%d", start);
	hs_assign("START", hs_genbuf[6]);
	hs_end();
	http_quit();
	return 0;
}

int record_compare(char *src, char *dst){
	int i;

	if(strlen(src) > strlen(dst))
		return 1;
	else if(strlen(src) < strlen(dst))
		return -1;
	for(i = 0; i < strlen(src); i++){
		if(*(src + i) > *(dst + i))
			return 1;
		else if(*(src + i) < *(dst + i))
			return -1;
	}
	return 0;
}

int bin_get_record(char *dir, char *file, struct fileheader *x, int size, int total)
{
	int fd, rt;
	int middle, left, right;
	
        if ((fd = open(dir, O_RDONLY, 0)) == -1) {
		return -1;
	}
	f_exlock(fd);
	left = 0, right = total - 1;
	while (left < right) {
		middle = (left + right)/2;
		if (lseek(fd, (off_t) ((middle+1) * size), SEEK_SET) == -1) {
			close(fd);
			return -1;
		}
		if (read(fd, x, size) != size) {
			close(fd);
			return -1;
		}
		rt = record_compare(file+2, x->filename+2);
		if (rt > 0) {
			left = middle + 1;
		} else if (rt == 0) {
			return middle + 1;
		} else {
			right = middle - 1;
		}
	}
	f_unlock(fd);
	close(fd);
	return -1;
}

int bin_mget_record(char *dir, char *file, struct fileheader *x, int size) {
	int middle, left, right;
	int fd;
	size_t rt;
	struct stat statbuf;
	struct fileheader *buf;

	if ((fd = open(dir, O_RDONLY, 0)) == -1) return -1;
	f_exlock(fd);
	if (fstat(fd, &statbuf) < 0) return -1;
	left = 0;
	right = statbuf.st_size/size - 1;
	if ((rt = (size_t)mmap(NULL, (size_t)statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == -1)
		return -1;
	buf = (struct fileheader *)rt;
	while (left <= right) {
		middle = (left + right) / 2;
		rt = record_compare(file+2, (buf+middle)->filename + 2);
		//printf("%d<%d<%d, r=%d, s=%s\n", left, middle, right, rt, (buf+middle)->filename);
		if (rt > 0) {
			left = middle + 1;
		} else if (rt == 0) {
			memcpy(x, buf+middle, size);
			f_unlock(fd);
			close(fd);
			return middle;
		} else {
			right = middle - 1;
		}
	}
	f_unlock(fd);
        close(fd);
        return -1;
}

int show_article() {
	char board[BFNAMELEN + 1], file[STRLEN], filename[STRLEN], dir[STRLEN];
	char owner[STRLEN], title[STRLEN], button[2][2048];
  char title_buf[TITLELEN * 6 + 1];
	struct fileheader x, x1, x2;
	struct boardheader *bentp;
	int total, start;
	FILE *fp;
	struct stat fs;
	
	init_all();
	modify_user_mode(READING);
	strsncpy(board, getparm("board"), 32);
	bentp = getbcache(board);
	if (bentp == 0) http_fatal("无效的讨论区");
	strlcpy(board, bentp->filename, sizeof(board));

	setboardfile(dir, board, ".DIR");
	total=file_size(dir)/sizeof(x);
	if(total<=0) http_fatal("本讨论区没有文章");
	
	/*babydragon: use parameter "file" instead of parameter "start"*/
	/* Pudding: bug fixed and backward-compatible */	
	if (!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	strlcpy(file, getparm("file"), sizeof(file));
	if (file[0] == '\0') {
		start = atoi(getparm("start"));
		if (start < 1) start = 1;
		if (start > total) start = total;
		if (get_record(dir, &x, sizeof(x), start) == -1)
			http_fatal("无法读取版面索引文件");
	} else {
		start = search_record(dir, &x, sizeof(x), cmpfilename, file);
		if (start == 0)	http_fatal("文章不存在或已被删除");
	}
	setboardfile(filename, board, file);
	if (stat(filename, &fs) == -1 || !S_ISREG(fs.st_mode))
		http_fatal("文章不存在或已被删除");
	if (strcmp(currentuser.userid, "guest")) {
		brc_initial(board);
		brc_addlist(file);
		brc_update();
	}

	hs_init(15);
	hs_setfile("bbscon.ptn");
        hs_assign("BOARD", board);
        snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "%d", getsecnum(board));
        hs_assign("SECNUM", hs_genbuf[0]);
        hs_assign("SECNAME", getsecname(board));
        hs_assign("BOARDNAME", bentp->title + 11);
	if( (fp = fopen(filename, "r")) == NULL)
		http_fatal("无法读取文章");
	if(fgets(owner, sizeof(owner), fp) != NULL){
		strtok(owner + 8, " ");
		hs_assign("AUTHOR", owner + 8);
	}else{
		hs_assign("AUTHOR", "");
	}
	if(fgets(title, sizeof(title), fp) != NULL){
    strlcpy(title_buf, void1(nohtml(title + 8)), sizeof(title_buf));
		hs_assign("TITLE", title_buf);
	}else{
		hs_assign("TITLE", "");
	}
	fclose(fp);
	hs_assign("FILE", file);
	hs_assign("ARTICLE", filename);
	/*
	if (start > 1)
		snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]), 
		    "<a href=bbscon?board=%s&start=1>第一篇</a> <a href=bbscon?board=%s&start=%d>上一篇</a>",
		    board, board, start - 1);
	else
		snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]),
		    "<font color=ff0000>第一篇 上一篇</font>");
	if (start < total) 
		snprintf(hs_genbuf[2], sizeof(hs_genbuf[2]),
		    "<a href=bbscon?board=%s&start=%d>下一篇</a> <a href=bbscon?board=%s&start=%d>最后一篇</a>",
		    board, start + 1, board, total);
	else
		snprintf(hs_genbuf[2], sizeof(hs_genbuf[2]),
		    "<font color=ff0000>下一篇 最后一篇</font>");
	*/
	if (start > 1
	&& get_record(dir, &x1, sizeof(x1), 1) != -1
	&& get_record(dir, &x2, sizeof(x2), start - 1) != -1) {
		snprintf(button[0], sizeof(button[0]),
			"<a href=bbscon?board=%s&file=%s>第一篇</a> <a href=bbscon?board=%s&file=%s>上一篇</a>",
			board, x1.filename, board, x2.filename);
	}
	else {
		snprintf(button[0], sizeof(button[0]), "<font color='red'>第一篇 上一篇</font>");
	}
	if (start < total
	&& get_record(dir, &x1, sizeof(x1), start + 1) != -1
	&& get_record(dir, &x2, sizeof(x2), total) != -1) {
		snprintf(button[1], sizeof(button[1]),
			"<a href=bbscon?board=%s&file=%s>下一篇</a> <a href=bbscon?board=%s&file=%s>最后一篇</a>",
			board, x1.filename, board, x2.filename);
	}
	else {
		snprintf(button[1], sizeof(button[1]), "<font color='red'>下一篇 最后一篇</font>");
	}
	hs_assign("PGBUTTONUP", button[0]);
	hs_assign("PGBUTTONDN", button[1]);
	sprintf(hs_genbuf[3], "%d", total);
	//sprintf(hs_genbuf[4], "%d", start);
	hs_assign("TOTAL", hs_genbuf[3]);
	hs_assign("START", "");
	hs_end();
	http_quit();
	return 0;
}

int show_brddigest() {
	char board[80], dir[80];
	int my_t_lines, start, total, fd, i;
	struct fileheader x;
	struct boardheader *x1;

 	init_all();
	strsncpy(board, getparm("board"), 32);
	my_t_lines=atoi(getparm("my_t_lines"));
	if (my_t_lines < 10 || my_t_lines > 40) my_t_lines = 20;
	x1=getbcache(board);
	if(x1==0) http_fatal("错误的讨论区");
	strcpy(board, x1->filename);
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	setboardfile(dir, board, ".DIGEST");
	fd = open(dir, O_RDONLY);
        if (fd == -1) http_fatal("错误的讨论区目录或本板没有文摘");
        total=file_size(dir)/sizeof(struct fileheader);
	if(total<=0) http_fatal("本讨论区目前没有文章");

	start=atoi(getparm("start"));
        if (start == 0 || start > total - 19) start = total - 19;
	if (start < 1) start = 1;

	hs_init(20);
	hs_setfile("bbsgdoc.ptn");
        hs_assign("BOARD", board);
        sprintf(hs_genbuf[0], "%d", getsecnum(board));
        hs_assign("SECNUM", hs_genbuf[0]);
        hs_assign("SECNAME", getsecname(board));
        hs_assign("BOARDNAME", x1->title + 11);
        hs_assign("BMLIST", x1->BM[0] ? userid_str(x1->BM) : "诚征版主中");
	hs_setloop("mainform");
	lseek(fd, (start - 1)*sizeof(struct fileheader), SEEK_SET);
	for(i=0; i<20; i++) {
		if (read(fd, &x, sizeof(x))<=0) break;
		sprintf(hs_genbuf[0], "%d", start+i);
		hs_assign("INDEX", hs_genbuf[0]);
		sprintf(hs_genbuf[1], "<a href=bbsgcon?board=%s&start=%d>%s%s</a>",
        		board, start+i, 
			strncmp(x.title, "Re: ", 4) ? "○ " : "",
			void1(nohtml(x.title)));
		hs_assign("TITLE", hs_genbuf[1]);
		hs_assign("FLAG", "g");
		hs_assign("AUTHOR", x.owner);
		sprintf(hs_genbuf[2], "%12.12s", 
			Cdtime(atoi(x.filename+2)));
		hs_assign("DATE", hs_genbuf[2]);
		hs_doloop("mainform");
      	}
	close(fd);
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
                sprintf(hs_genbuf[4], "<a href=bbsgdoc?board=%s&start=1>首页</a> <a href=bbsgdoc?board=%s&start=%d>上一页</a>",
                        board, board, start - my_t_lines);
        hs_assign("PGBUTTONUP", hs_genbuf[4]);
        if (start >= total - my_t_lines + 1)
                strcpy(hs_genbuf[5], "<font color=ff0000>下一页 末页</font>");
        else
                sprintf(hs_genbuf[5], "<a href=bbsgdoc?board=%s&start=%d>下一页</a> <a href=bbsgdoc?board=%s&start=%d>末页</a>",
                        board, start + my_t_lines, board, total - my_t_lines + 1);
        hs_assign("PGBUTTONDN", hs_genbuf[5]);
	hs_end();
	http_quit();
	return 0;
}

int show_digest() {
	char board[BFNAMELEN + 1], dir[STRLEN], filename[STRLEN];
	struct fileheader x;
	struct boardheader *bentp;
	int start, total;

	init_all();
	strsncpy(board, getparm("board"), 32);
	bentp = getbcache(board);
	if (bentp == 0) http_fatal("错误的讨论区");
	strlcpy(board, bentp->filename, sizeof(board));
	start=atoi(getparm("start"));
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");

	setboardfile(dir, board, ".DIGEST");
	total=file_size(dir)/sizeof(x);
	if(total<=0) http_fatal("此讨论区不存在或者为空");

	if (start > total) start = total;
	if (start < 1) start = 1;
	get_record(dir, &x, sizeof(x), start);
	setboardfile(filename, board, x.filename);
	if (!file_exist(filename)) http_fatal("本文不存在或者已被删除");

	hs_init(10);
	hs_setfile("bbsgcon.ptn");
        hs_assign("BOARD", board);
        snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "%d", getsecnum(board));
        hs_assign("SECNUM", hs_genbuf[0]);
        hs_assign("SECNAME", getsecname(board));
        hs_assign("BOARDNAME", bentp->title + 11);
	hs_assign("TITLE", void1(x.title));
	hs_assign("ARTICLE", filename);
	if (start > 1)
		snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]), 
		    "<a href=bbsgcon?board=%s&start=1>第一篇</a> <a href=bbsgcon?board=%s&start=%d>上一篇</a>",
		    board, board, start - 1);
	else
		snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]),
		    "<font color=ff0000>第一篇 上一篇</font>");
	if (start < total) 
		snprintf(hs_genbuf[2], sizeof(hs_genbuf[2]),
		    "<a href=bbsgcon?board=%s&start=%d>下一篇</a> <a href=bbsgcon?board=%s&start=%d>最后一篇</a>",
		    board, start + 1, board, total);
	else
		snprintf(hs_genbuf[2], sizeof(hs_genbuf[2]),
		    "<font color=ff0000>下一篇 最后一篇</font>");
	hs_assign("PGBUTTONUP", hs_genbuf[1]);
	hs_assign("PGBUTTONDN", hs_genbuf[2]);
	sprintf(hs_genbuf[3], "%d", total);
	sprintf(hs_genbuf[4], "%d", start);
	hs_assign("TOTAL", hs_genbuf[3]);
	hs_assign("START", hs_genbuf[4]);
	hs_end();
	http_quit();
	return 0;
}

int show_board_topic() {
	struct boardheader *x1;
	struct fileheader *data;
	int my_t_lines;
	char board[BFNAMELEN + 1];
	int total, total2=0, start;
	char buf[7][255];
	int fd;
	char dir[80];
	int i, sum=0;
 	init_all();
	strsncpy(board, getparm("board"), 32);
	my_t_lines = atoi(getparm("my_t_lines"));
	if (my_t_lines == 0) my_t_lines = 20;
	x1=getbcache(board);
	if(x1==0) http_fatal("错误的讨论区");
	strlcpy(board, x1->filename, sizeof(board));
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	setboardfile(dir, board, DOT_DIR);
	fd = open(dir, O_RDONLY);
	if (fd == -1) http_fatal("错误的讨论区目录");
	total=file_size(dir)/sizeof(struct fileheader);
	data=calloc(sizeof(struct fileheader), total);
	if(data==0) http_fatal("内存溢出");
	read (fd, data, sizeof(struct fileheader) * total);
	close(fd);
	if(total<=0) http_fatal("本讨论区目前没有文章");
	for (i=0; i<total; i++) 
		if (data[i].id == atoi(data[i].filename + 2)) total2++;
	start=atoi(getparm("start"));
        if (start == 0 ||
		start > total2 - 19) start = total2 - 19;
  	if (start < 1) start = 1;
//	printf("<nobr><center>\n");

	hs_init(20);
	hs_setfile("bbstdoc.ptn");
	hs_assign("BOARD", board);
	sprintf(buf[0], "%d", getsecnum(board));
	hs_assign("SECNUM", buf[0]);
	hs_assign("SECNAME", getsecname(board));
	hs_assign("BOARDNAME", x1->title + 11);
	hs_assign("BMLIST", x1->BM[0] ? userid_str(x1->BM) : "诚征版主中");
	if (has_BM_perm(&currentuser, board)) hs_assign("HAS_BM_PERM", "");
	if (loginok) hs_assign("LOGINOK", "");

	hs_setloop("mainform");
	for(i=0; i<total; i++) {
		if (atoi(data[i].filename + 2) != data[i].id) continue;
//		if(!strncmp(data[i].title, "Re:", 3)) continue;
		sum++;
		if(sum < start) continue;
		if(sum > start+19) break;
		sprintf(buf[0], "%d", sum);
		hs_assign("INDEX", buf[0]);
		sprintf(buf[1], "<font color=#909090>%s</font>",
			flag_str(data[i].flag)[0]?
			flag_str(data[i].flag):"&nbsp");
		hs_assign("FLAG", buf[1]);
		hs_assign("TITLE", nohtml(data[i].title));
		hs_assign("FILE", data[i].filename);
		hs_assign("AUTHOR", data[i].owner);
		hs_assign("REPLY", topic_stat(data, i, total));
		sprintf(buf[5], "%12.12s", 
			Cdtime(atoi(data[i].filename+2)));
		hs_assign("DATE", buf[5]);
		hs_doloop("mainform");
      	}
	free(data);
        sprintf(buf[0], "%d", total2);
        hs_assign("TOTAL", buf[0]);
        sprintf(buf[1], "%d", my_t_lines);
        hs_assign("PERPAGE", buf[1]);
        sprintf(buf[2], "%d", (start - 1) / my_t_lines + 1);
        hs_assign("PAGE", buf[2]);
        sprintf(buf[3], "%d", (total2 - 1) / my_t_lines + 1);
        hs_assign("PGTOTAL", buf[3]);
        if (start == 1)
                strcpy(buf[4], "<font color=ff0000>首页 上一页</font>");
        else
                sprintf(buf[4], "<a href=bbstdoc?board=%s&start=1>首页</a> <a href=bbstdoc?board=%s&start=%d>上一页</a>",
                        board, board, start - my_t_lines);
        hs_assign("PGBUTTONUP", buf[4]);
        if (start >= total2 - my_t_lines + 1)
                strcpy(buf[5], "<font color=ff0000>下一页 末页</font>");
        else
                sprintf(buf[5], "<a href=bbstdoc?board=%s&start=%d>下一页</a> <a href=bbstdoc?board=%s&start=%d>末页</a>",
                        board, start + my_t_lines, board, total2 - my_t_lines + 1);
        hs_assign("PGBUTTONDN", buf[5]);
        hs_end();
	http_quit();
	return 0;
}

char *topic_stat(struct fileheader *data, int from, int total) {
	static char buf[256];
	int i, re=0;
	for(i=from + 1; i<total; i++) {
		if(data[from].id == data[i].id) {
			re++;
		}
	}
	sprintf(buf, "%d", re);
	return buf;
}

int cmpfilename(void *filename, void *fh) {
	return (!strcmp(((struct fileheader *)fh)->filename, (char *)filename));
}

int cmparticleid(void *id, void *fh) {
	return (((struct fileheader *)fh)->id == *(int *)id);
}

int show_topic() {
	char board[BFNAMELEN + 1], dir[STRLEN], 
		file[STRLEN];
	struct fileheader x;
	struct boardheader *bentp;
	int num=0, total, id, count;
  char title_buf[TITLELEN * 6 + 1];
	init_all();
	strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("file"), 32);
	bentp = getbcache(board);
	if (bentp == NULL) http_fatal("错误的讨论区");
	strlcpy(board, bentp->filename, sizeof(board));
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if(loginok) brc_initial(board);
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2)) http_fatal("错误的参数1");
	if(strstr(file, "..") || strstr(file, "/")) http_fatal("错误的参数2");
	setboardfile(dir, board, ".DIR");
	if (search_record(dir, &x, sizeof(x), cmpfilename, file) == 0) {
		http_fatal("本文件不存在");
	}

	hs_init(10);
	hs_setfile("bbstcon.ptn");
        hs_assign("BOARD", board);
        snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]), "%d", getsecnum(board));
        hs_assign("SECNUM", hs_genbuf[0]);
        hs_assign("SECNAME", getsecname(board));
        hs_assign("BOARDNAME", bentp->title + 11);

	id = x.id;
	num = search_record(dir, &x, sizeof(x), cmparticleid, &id);

	if (num == 0) http_fatal("不存在的主题");

	count = 1;
	hs_assign("AUTHOR", x.owner);
  strlcpy(title_buf, void1(nohtml(x.title)), sizeof(title_buf));
	hs_assign("TITLE", title_buf);
	hs_assign("FILE", x.filename);
	setboardfile(file, board, x.filename);
	hs_assign("ARTICLE", file);
	hs_setloop("mainform");
	hs_doloop("mainform");

	total = get_num_records(dir, sizeof(x));
	while (++num <= total) {
		get_record(dir, &x, sizeof(x), num);
		if (x.id == id) {
			count ++;
			if(loginok) brc_addlist(x.filename);
			hs_assign("AUTHOR", x.owner);
  		strlcpy(title_buf, void1(nohtml(x.title)), sizeof(title_buf));
			hs_assign("TITLE", title_buf);
			hs_assign("FILE", x.filename);
			setboardfile(file, board, x.filename);
			hs_assign("ARTICLE", file);
			hs_doloop("mainform");
		}
	}

	if(loginok) brc_update();
	snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]), "%d", count);
	hs_assign("TOTAL", hs_genbuf[1]);
	hs_end();
	http_quit();
	return 0;
}

void show_file_stuff(char *board, struct fileheader *x) {
	FILE *fp;
	char path[80], buf[512];
	if(loginok) brc_addlist(x->filename);
	sprintf(path, "boards/%s/%s", board, x->filename);
	fp=fopen(path, "r");
	if(fp==0) return;
	article_read_header(board, x);
	while(1) {
		if(fgets(buf, 500, fp)==0) break;
		if (!strncmp(buf, ": ",2)) printf("<font color=808080>");
		hhprintf("%s", buf);
		if (!strncmp(buf, ": ",2)) printf("</font>");
	}
	fclose(fp);
	article_read_foot();
}


int show_all_boards() {
	int i, total;
	struct boardheader data[MAXBOARD], *x;
	char *ptr,  buf1[100], buf[10][255];
	init_all();
	modify_user_mode(READBRD);
	total = 0;
	for(i=0; i<MAXBOARD; i++) {
		x=&(shm_bcache->bcache[i]);
		if(x->filename[0]<=32 || x->filename[0]>'z') continue;
		if(!has_read_perm(&currentuser, x->filename)) continue;
		memcpy(&data[total], x, sizeof(struct boardheader));
		total++;
	}
	qsort(data, total, sizeof(struct boardheader), cmpboard);
	hs_init(20);
	hs_setfile("bbsall.ptn");
	hs_setloop("mainform");
	for(i=0; i<total; i++) {
		ptr=strtok(data[i].BM, " ,;");
		sprintf(buf1, "boards/%s/.DIR", data[i].filename);
		hs_assign("READFLAG",
	      		board_read(data[i].filename) ? "◇" : "<font class='new'>◆</font>");   
		hs_assign("VOTEFLAG", (data[i].flag & VOTE_FLAG)?
				"<font color='#ff0000'>V</font>":"");
		sprintf(buf[1], "%6.6s", data[i].title + 1);
		hs_assign("TYPE", buf[1]);
		hs_assign("BOARD", data[i].filename);
		hs_assign("BOARDNAME", data[i].title + 7);
		hs_assign("AUTHOR", (ptr)?userid_str(ptr):"诚征版主中");
		sprintf(buf[5], "%d", 
			filenum(data[i].filename));
		hs_assign("TOTAL", buf[5]);
		sprintf(buf[6], "%12.12s", Cdtime(file_time(buf1)));
		hs_assign("DATE", buf[6]);
		sprintf(buf[7], "%d", getsecnum(data[i].filename));
		hs_assign("SECNUM", buf[7]);
		hs_doloop("mainform");
	}
	hs_end();
	http_quit();
	return 0;
}

int select_board() {
	char *board, buf[STRLEN], *board1, *title;
	int i, total=0;
	init_all(); 
	board=nohtml(getparm("board"));
	if(board[0]==0) {
		hs_init(1);
		hs_setfile("search_brd.ptn");
		hs_end();
		http_term();
	} else {
		for(i=0; i<MAXBOARD; i++) {
			board1=shm_bcache->bcache[i].filename;
			if(!has_read_perm(&currentuser, board1)) continue;
			if(!strcasecmp(board, board1)) {
				sprintf(buf, "bbsdoc?board=%s", board1);
				redirect(buf);
				http_term();
			}
		}
		printf("%s -- 选择讨论区<hr color=green>\n", BBSNAME);
		printf("找不到 %s 讨论区", board);
		printf("标题中含有'%s'的讨论区有: <br><br>\n", board);
		printf("<table>");
		for(i=0; i<MAXBOARD; i++) {
			board1=shm_bcache->bcache[i].filename;
			title=shm_bcache->bcache[i].title;
			if(!has_read_perm(&currentuser, board1)) continue;
			if(strcasestr(board1, board) || strcasestr(title, board)) {
				total++;
				printf("<tr><td>%d", total);
				printf("<td><a href=bbsdoc?board=%s>%s</a><td>%s<br>\n",
					board1, board1, title+7);
			}
		}
		printf("</table><br>\n");
		printf("共找到%d个符合条件的讨论区.\n", total);
	}
	http_quit();
	return 0;
}

int show_note(char *board) {
	char filename[STRLEN], recfile[STRLEN];
	//FILE *fp;
	struct stat st;

	setvotefile(filename, board, "notes");
	if (stat(filename, &st) != -1) {
		if (st.st_mtime < (time(NULL) - 7 * 86400)) {
			utimes(filename, NULL);
			setvotefile(recfile, board, "noterec");
			unlink(recfile);
		}
	} else {
		return -1;
	}

#ifndef ALWAYS_SHOW_BRDNOTE
	if (note_flag(board, '\0') == 0) note_flag(board, 'R');
	else return 0;
#endif

	printf("<SCRIPT LANGUAGE=javascript>window.open('bbsnot?board=%s', '%s版备忘录');</SCRIPT>", 
		board, board);

	return 0;
}

int show_board_note() {
	FILE *fp;
	char buf[512], board[80], filename[80];
	struct boardheader *x1;
	
	init_all();
	strsncpy(board, getparm("board"), 32);
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的版面");
	if((x1=getbcache(board)) == NULL)
		http_fatal("错误的版面");
	//printf("%s -- 备忘录 [讨论区: %s]<hr color=green>\n", BBSNAME, board);
 	sprintf(filename, "vote/%s/notes", board);
	fp=fopen(filename, "r");
	if(fp==0) {
		http_fatal("本讨论区尚无「进版画面」。\n");
		http_term();
	}

	printf("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>"
		"<html>"
		"<head>"
		"<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
		"<title>%s</title>"
		"<link href='templates/global.css' rel='stylesheet' type='text/css' />"
		"<style type='text/css'>"
		"body{        margin-right:16px;          }"
		"</style>"
		"</head>"
		"<body>"
		"<div id='head'>"
		"<div id='location'>"
		"<p><img src='images/location01.gif' alt='' align='absmiddle'/><a href='bbssec'>%s</a></p>"
		"<p><img src='images/location02.gif' alt='' align='absmiddle'/><a href='bbsboa?board=%d'>%s</a></p>"
		"<p><img src='images/location03.gif' alt='' align='absmiddle'/><a href='bbsdoc?board=%s'>%s</a></p>"
		"</div>"
		"</div>",
		BBSNAME, BBSNAME, getsecnum(board), getsecname(board), board, x1->title + 11);
	printf("<table border='0' cellpadding='0' cellspacing='0' class='table'>"
		"<tr>"
		"<td colspan='2' class='tb_head'>"
		"<img src='images/table_ul02.gif' alt='' width='37' height='25' align='absmiddle' class='tb_ul' style='float:left' />"
		"<div style='margin-top:8px'>%s版备忘录</div></td>"
		"<td width='19' align='right' valign='top' class='tb_r'>"
		"<img src='images/table_ur.gif' alt=''/></td>"
		"</tr>"
		"<tr>"
		"<td class='tb_l'></td>"
		"<td>"
		"<center><pre>", board);
	while(1) {
		char *s;
		memset(buf, 0, sizeof(buf));
		if(fgets(buf, 512, fp)==0) break;
		while(1) {
			int i;
			s=strstr(buf, "$userid");
			if(s==0) break;
			for(i=0; i<7; i++) s[i]=32;
			for(i=0; i<strlen(currentuser.userid); i++)
				s[i]=currentuser.userid[i];
		}
		hhprintf("%s", buf);
	}
 	fclose(fp);
	printf("</pre></center>"
		"</td>"
		"<td class='tb_r'> </td>"
		"</tr>"
		"<tr class='tb_bottom'>"
		"<td width='25'><img src='images/table_bl.gif' alt=''/></td>"
		"<td width='1063'></td>"
		"<td align='right'><img src='images/table_br.gif' alt=''/></td>"
		"</tr>"
		"</table>"
		"<br/>"
		"<center>"
		"<a href='bbsdoc?board=%s'>[本讨论区]</a>"
		"</center>"
		"</form>"
		"</body>"
		"</html>", board);	
//	if(has_BM_perm(&currentuser, board)) 
//		printf("[<a href=bbsmnote?board=%s>编辑进版画面</a>]", board);
	http_quit();
	return 0;
}
/*
void brdfind_form(char *board) {
	struct title_t title;
	struct boardheader *bh;
	bh = getbcache(board);
	if (bh) board = bh->filename;
	title.depth=1;
	title.board="版面文章查询";
	form_title(&title);
	printf("<table><form action=bbsbfind?type=1 method=post>\n");
	printf("<tr><td>版面名称: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n", board);
	printf("<tr><td>标题含有: <input type=text maxlength=50 size=20 name=title> AND ");
	printf("<input type=text maxlength=50 size=20 name=title2>\n");
	printf("<tr><td>标题不含: <input type=text maxlength=50 size=20 name=title3>\n");
	printf("<tr><td>作者帐号: <input type=text maxlength=12 size=12 name=userid><br>\n");
	printf("<tr><td>时间范围: <input type=text maxlength=4  size=4  name=dt value=7> 天以内<br>\n");
	printf("<tr><td>精华文章: <input type=checkbox name=mg> ");
	printf("不含跟贴: <input type=checkbox name=og><br><br>\n");
	printf("<tr><td><input type=submit value=递交查询结果>\n");
	printf("</form></table>");
//	printf("[<a href='bbsdoc?board=%s'>返回上一页</a>] ", board);
//	printf("[<a href=bbsfind>全站文章查询</a>]");
	http_term();
}*/

int find_inboard() {
	int fd;
	int num=0, total=0, type, dt, mg=0, og=0;
	char dir[80], title[TITLELEN + 1], title2[80], title3[80];
	char board[BFNAMELEN + 1], userid[IDLEN + 1];
	struct title_t title1;
	char *href[6], buf[5][255];
	int width[]={32,32,80,90,0}, i;
	struct boardheader *brd;
	struct fileheader x;
	init_all();
	type=atoi(getparm("type"));
	strsncpy(board, getparm("board"), 30);
	if (type == 0) {
		hs_init(1);
		hs_setfile("bbsbfind.ptn");
		hs_assign("BOARD", board);
		hs_end();
		return 0;
	}
	//hs_init(20);
	//hs_setfile("bfind.ptn");
	strsncpy(title, getparm("title"), 60);
	strsncpy(title2, getparm("title2"), 60);
	strsncpy(title3, getparm("title3"), 60);
	strsncpy(userid, getparm("userid"), 60);
	dt=atoi(getparm("dt"));
	if(!strcasecmp(getparm("mg"), "on")) mg=1;
	if(!strcasecmp(getparm("og"), "on")) og=1;
	if(dt<0) dt=0;
	if(dt>9999) dt=9999;
	brd=getbcache(board);
	if(brd==0) http_fatal("错误的讨论区");
	strlcpy(board, brd->filename, sizeof(board));
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区名称");
	title1.depth=3;
	title1.board=board;
	title1.num=title1.BM=0;
	form_title(&title1);
	setboardfile(dir, board, ".DIR");
	fd = open(dir, O_RDONLY);
	if (fd == -1) http_fatal("讨论区错误或没有目前文章");
	href[0]="编号";
	href[1]="标记";
	href[2]="作者";
	href[3]="日期";
	href[4]="标题";
	href[5]=NULL;
	form_header(href, width);
	for (i=0; i<5; i++) href[i]=buf[i];
	while(1) {
		if (read(fd, &x, sizeof(x)) <= 0) break;
		num++;
		if(title[0] && !strcasestr(x.title, title)) continue;
		if(title2[0] && !strcasestr(x.title, title2)) continue;
		if(userid[0] && strcasecmp(x.owner, userid)) continue;
		if(title3[0] && strcasestr(x.title, title3)) continue;
		if(abs(time(0)-atoi(x.filename+2))>dt*86400) continue;
		if(mg && !(x.flag & FILE_MARKED) && !(x.flag & FILE_DIGEST)) continue;
		if(og && !strncmp(x.title, "Re: ", 4)) continue;
		total++;
		sprintf(buf[0], "%d", num);
		sprintf(buf[1], "&nbsp;%s&nbsp;", flag_str(x.flag));
		sprintf(buf[2], "%s", userid_str(x.owner));
		sprintf(buf[3], "%12.12s", Cdtime(atoi(x.filename+2)));
		sprintf(buf[4], "<a href=bbscon?board=%s&file=%s&start=%d>%40.40s </a>",board, x.filename,num,x.title);
		form_content(href, width, 4);
		if(total>=999) break;
	}
	close(fd);
	printf("</table><br><font color=#0099cc>");
	if (total==0) {
		printf("无法找到符合条件的文章");
	} else {
		printf("以上是讨论区 <font color=red>%s</font> 内, ", board);
		if(strlen(nohtml(title))) printf("标题含 %s ", nohtml(title));
		if(title2[0]) printf("和 %s ", nohtml(title2));
		if(title3[0]) printf("不含 %s ", nohtml(title3));
		printf("作者为 <font color=red>%s</font>, <font color=red>%d</font> 天以内的%s文章.\n", 
			userid[0] ? userid_str(userid) : "所有作者", dt, mg ? "精华" : "所有");
		printf("共找到 <font color=red>%d</font> 篇文章符合条件", total);
		if(total>999) printf("(匹配结果过多, 省略第1000以后的查询结果)");
	}
	printf("</font>");
	http_quit();
	return 0;
}

int clear_read_flag() {
	char board[80], start[80], buf[256];
	init_all();
	strsncpy(board, getparm("board"), 32);
	strsncpy(start, getparm("start"), 32);
	if(!loginok) http_fatal("匆匆过客无法执行此项操作, 请先登录");
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	brc_initial(board);
	brc_clear();
	brc_update();
	sprintf(buf, "bbsdoc?board=%s&start=%d", board, atoi(start));
	refreshto(buf, 0);
	http_quit();
	return 0;
}

/** XXX: this function is incorrect */
int site_search() {
	char user[32], title3[80], title[80], title2[80];
	int day;
	init_all();
	if (!HAS_PERM(PERM_OBOARDS) && !HAS_PERM(PERM_INTERNAL)) http_fatal("你无权使用本功能");
  	day=atoi(getparm("day"));
  	if(day==0) {
		hs_init(1);
		hs_setfile("search_art.ptn");
		hs_end();
	} else if (day > 7) {
		http_fatal("sorry, days have a maximal value of 7");
	} else {
		search_site(user, title, title2, title3, day*86400);
	}
	http_quit();
	return 0;
}

int search_site(char *id, char *pat, char *pat2, char *pat3, int dt) {
	int fd;
  	char board[BFNAMELEN + 1], dir[256];
	int total, now=time(0), i, sum=0, n, t;
	int width[]={32, 80, 0, 100, 60};
	char *href[6], buf[5][255];
	struct fileheader x;
	href[0]="编号";
	href[1]="作者";
	href[2]="标  题";
	href[3]="发表版面";
	href[4]="日期";
	href[5]=NULL;
	form_header(href, width);
	for (i=0; i<5; i++) href[i]=buf[i];
	for(i=0; i<MAXBOARD; i++) {
		total=0;
		strlcpy(board, shm_bcache->bcache[i].filename, sizeof(board));
		if(!has_read_perm(&currentuser, board)) continue;
		setboardfile(dir, board, ".DIR");
		fd = open(dir, O_RDONLY);
		if (fd == -1) continue;
		n=0;
		while(1) {
			n++;
			if (read(fd, &x, sizeof(x)) <= 0) break;
			t=atoi(x.filename+2);
			if(id[0]!=0 && strcasecmp(x.owner, id)) continue;
			if(pat[0] && !strcasestr(x.title, pat)) continue;
			if(abs(now-t)>dt) continue;
			if(pat2[0] && !strcasestr(x.title, pat2)) continue;
			if(pat3[0] && strcasestr(x.title, pat3)) continue;
			sprintf(buf[0], "%d", n);
			sprintf(buf[1], "%s", userid_str(x.owner));
			sprintf(buf[2], "<a href=bbscon?board=%s&file=%s&start=%d>%s</a>", 
				board, x.filename, n, nohtml(x.title));
			sprintf(buf[3], "<a href=bbsdoc?board=%s>%s</a>", 
				board, board);
			sprintf(buf[4], "%6.6s", Cdtime(atoi(x.filename+2)));
			form_content(href, width, 2);
			total++;
			sum++;
			if(sum>1999) {
				printf("</table>");
				http_term();
			}
		}
		printf("</table>\n");
		if(total==0) continue;
	}
	printf("<br><font color=#0099cc>");
	printf("以上为本站 <font color=red>%d</font> 天内，", dt/86400);
	if (strlen(id)) printf("作者为 <font color=red>%s</font> ", 
		userid_str(id));
	if (strlen(pat)) printf("标题含有 <font color=red>%s</font> ",
		nohtml(pat));
	if (pat2[0]) printf("和 <font color=red>%s</font> ", nohtml(pat2));
	if (pat3[0]) printf("不含 <font color=red>%s</font> ", nohtml(pat3));
	printf("的文章，一共找到 <font color=red>%d</font> 篇文章符合查找条件</font><br>\n", 
		sum);
	return 0;
}

#ifndef ALWAYS_SHOW_BRDNOTE
/* Henry: borrowed from telnet code vote_flag() */
int note_flag(char *bname, char val) {
        char buf[STRLEN], flag;
        int fd, num, size;

        num = getusernum(currentuser.userid);
        setvotefile(buf, bname, "noterec");        /* 讨论区备忘录的旗标 */
        if (num >= MAXUSERS) return -1;
        if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1) return -1;
        f_exlock(fd);
        size = (int) lseek(fd, 0, SEEK_END);
        memset(buf, 0, sizeof (buf));
        while (size <= num) {
                write(fd, buf, sizeof (buf));
                size += sizeof (buf);
        }
        lseek(fd, (off_t) num, SEEK_SET);
        read(fd, &flag, 1);
        if ((flag == 0 && val != 0)) {
                lseek(fd, (off_t) num, SEEK_SET);
                write(fd, &val, 1);
        }
        f_unlock(fd);
        close(fd);
        return flag;
}
#endif
