#include "webbs.h"

int show_annpath() {
	char *board;
	int total;
        FILE *fp;
	int i, j, k;
	struct boardheader *bhp;
	struct annheader ah;
	char fname[512], path[512], secname[STRLEN];
	char nbsptemp[40];
	char buf[6][255];

        init_all();
	modify_user_mode(DIGEST);
        strsncpy(path, getparm("path"), 490);
	if(strstr(path, "..")) http_fatal("此目录不存在");
	total = 0;

	hs_init(20);
	hs_setfile("bbs0an.ptn");
	if (strncmp(path, "@GROUP:", 7)) {
		board=getbfroma(path);
		if (board[0] && !has_read_perm(&currentuser, board))
			http_fatal("目录不存在");
		snprintf(fname, sizeof(fname) - 6, "0Announce/%s", 
			path);
		if (!file_isdir(fname)) http_fatal("目录不存在");
		strcat(fname, "/.DIR");
		total = file_size(fname) / sizeof(ah);
		if (total == 0) http_fatal("目前没有文章");

		if (board[0]) {
			hs_assign("BOARD", board);
			sprintf(secname, "%s", getsecname(board));
			hs_assign("SECNAME", secname);
			snprintf(hs_genbuf[0], sizeof(hs_genbuf[0]),
					"%d", getsecnum(board));
			hs_assign("SECID", hs_genbuf[0]);
		} else {
			hs_assign("ANNOUNCE", "精华区公布栏");
		}
	} else {	//分类讨论区
		if (strlen(path) < 9) http_fatal("非法讨论区");
	}

	if (!strncmp(path, "@GROUP:", 7)) {	//分类讨论区
		hs_setloop("mainform");
		j = 0;
		for (i = 0; i < MAXBOARD; i++) {
			bhp = &(shm_bcache->bcache[i]);
			if(bhp->filename[0]<=32 || bhp->filename[0]>'z')
				continue;
			if(!has_read_perm(&currentuser, bhp->filename))
				continue;
			if(path[7] != '*' && !strchr(seccode[path[7]-'0'], 
			    bhp->title[0]))
				continue;
			j++;
			sprintf(buf[0], "%d", j);
			hs_assign("INDEX", buf[0]);
			hs_assign("TYPE", "目录");
			sprintf(buf[2], "<a href=bbs0an?path=boards/%s>%s</a>",
				bhp->filename, nbsp(bhp->title+8));
			hs_assign("TITLE", buf[2]);
			hs_assign("BM", "");
			sprintf(buf[4], "%12.12s", Cdtime(time(0)));
			hs_assign("TIME", buf[4]);
			hs_doloop("mainform");
		}
		/*
		hs_assign("INDEX", "*");
		hs_assign("TYPE", "衔接");
		hs_assign("TITLE", "<a href=javascript:history.go(-1)>返回上一层</a>");
		hs_assign("BM", "");
		snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]), "%12.12s", Cdtime(time(0)));
		hs_assign("DATE", hs_genbuf[1]);
		hs_doloop("mainform");*/
		hs_end();
		http_quit();

		return 0;
	}

	fp = fopen(fname, "r");
	hs_setloop("mainform");
        for(i=0; i<total; i++) {
		fread(&ah, sizeof(ah), 1, fp);

		/* check read permission */

		snprintf(buf[0], sizeof(buf[0]), "%d", i + 1);
		hs_assign("INDEX", buf[0]);

		if (ah.flag & ANN_DIR)
			hs_assign("TYPE", "目录");
		else if (ah.flag & ANN_FILE)
			hs_assign("TYPE", "文件");
		else if (ah.flag & ANN_LINK)
			hs_assign("TYPE", "衔接");
		else if (ah.flag & ANN_READONLY)
			hs_assign("TYPE", "只读");
		else if (ah.flag & ANN_GUESTBOOK)
			hs_assign("TYPE", "留言");
		else if (ah.flag & ANN_PERSONAL)
			hs_assign("TYPE", "文集");
		else	hs_assign("TYPE", "错误");

		if (!strcmp(ah.filename, "@NULL")) {
			snprintf(buf[1], sizeof(buf[1]), "%37.37s", nohtml(ah.title));
		}
		else if (!strcmp(ah.filename, "@BOARDS")) {
			snprintf(buf[1], sizeof(buf[1]), 
				"<a href=bbs0an?path=boards/%s>%37.37s</a>",
				ah.owner, nohtml(ah.title));
		}
		else if (!strncmp(ah.filename, "@GROUP", 6)) {
			snprintf(buf[1], sizeof(buf[1]), 
				"<a href=bbs0an?path=%s%s>%37.37s</a>",
				ah.filename, ah.title, ah.title);
			/* 格式: "@GROUP:[0-9, A-Z, *]标题" */
		}
		else {
			snprintf(fname, sizeof(fname), 
				"0Announce/%s/%s", path, 
				ah.filename);
			if (!file_exist(fname))
				snprintf(buf[1], sizeof(buf[1]), 
					"%s", nohtml(ah.title));
			else {
				strlcpy(nbsptemp, ah.title, 38);
				k = strlen(nbsptemp) - 1;
				while (nbsptemp[k] == ' ') nbsptemp[k--] = 0;
				snprintf(buf[1], sizeof(buf[1]), 
					"<a href=%s?path=%s/%s>%s</a>",
					file_isdir(fname) ? "bbs0an" : "bbsanc",
      		 			path, ah.filename, nbsp(nohtml(nbsptemp)));
			}
		}
		hs_assign("TITLE", buf[1]);
	
		hs_assign("BM", (ah.filename[0] == '@' || ah.editor[0] == 0) ?
			"":userid_str(ah.editor));
		snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]), "%12.12s", Cdtime(ah.mtime));
		hs_assign("DATE", hs_genbuf[1]);
		hs_doloop("mainform");
	}
	/*
	hs_assign("INDEX", "*");
	hs_assign("TYPE", "衔接");
	hs_assign("TITLE", "<a href=javascript:history.go(-1)>返回上一层</a>");
	hs_assign("BM", "");
	snprintf(hs_genbuf[1], sizeof(hs_genbuf[1]), "%12.12s", Cdtime(time(0)));
	hs_assign("DATE", hs_genbuf[1]);
	hs_doloop("mainform");*/

	hs_end();
        http_quit();
	return 0;
}

int show_annfile() {
	FILE *fp;
	char *board, path[512], buf[512]; 
	init_all();
	modify_user_mode(DIGEST);
	strsncpy(path, getparm("path"), 511);
	board=getbfroma(path);
	buf[0]=0;
	if(board[0]) sprintf(buf, "%s版", board);
	sprintf(buf, "0Announce/%s", path);
	fp=fopen(buf, "r");
	if(fp==0) http_fatal("错误的文件名");
	
	printf("<html>"
		"<head>"
		"<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
		"<title>%s</title>"
		"<link href='templates/global.css' rel='stylesheet' type='text/css' />"
		"<style>"
		 " body{       margin-right:16px;	  }"
		"</style>"
		"</head>"
		"<body>"
		"<form name='form1' method='post' action=''>"
		"<div id='head'>"
		"<div id='location'>"
		"<p><img src='images/location01.gif' alt='' align='absmiddle'/><a href='bbssec'>%s</a></p>"
		"<p><img src='images/location03.gif' alt='' align='absmiddle' />%s精华区文章阅读</p>"
		"</div>"
		"</div>"
		"<table border='0' cellpadding='0' cellspacing='0' class='table'>"
		"<tr>"
		"<td colspan='2' class='tb_head'>"
		"<img src='images/table_ul05.gif' align='absmiddle' style='float:left; margin-left:0!important;margin-left:-3px;'/>"
		"<div class='title'>"
		"<img src='images/li01.gif' width='9' height='9' align='absmiddle'/>%s精华区文章阅读</div></td>"
		"<td align='right' class='tb_head'><img src='images/table_ur.gif' alt=''/></td>"
		"</tr>"
		"<tr>"
		"<td class='tb_l'> </td>"
		"<td class='tb_r'> </td>"
		"</tr>"
		"<tr>"
		"<td class='tb_l'> </td>"
		"<td class='border content2'><pre>", 
		BBSNAME, BBSNAME, board, board);
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		hhprintf("%s", void1(buf));
	}
	fclose(fp);
	printf("</pre></td>" 
		"<td class='tb_r'> </td>"
		"</tr>"
		"<tr> "
		"<td class='tb_l'> </td>"
		"<td class='footer'>※ 来源:．" BBSNAME "电子公告板系统（BBS） " BBSWWWDOMAIN "</td>"
		"<td class='tb_r'> </td>"
		"</tr>"
		"<tr class='tb_bottom'> "
		"<td><img src='images/table_bl.gif' alt=''/></td>"
		"<td></td>"
		"<td align='right'><img src='images/table_br.gif' alt=''/></td>"
		"</tr>"
		"</table>"
		"<div id='footer'> "
		"<p><a href='javascript:history.go(-1)'>返回上一页</a> <a href=bbsdoc?board=%s>本讨论区</a>"
		"</p>"
		"</div>"
		"</form>"
		"</body>"
		"</html>", board);
	http_quit();
	return 0;
}

char *
anno_path_of (board)
char *board; {
        static char buf[256];
        if (board == NULL || board[0] == '\0') return "";
        snprintf(buf, sizeof(buf), "boards/%s", board);
        return buf;
}

char *
getbfroma (path)
char *path; {
        static char bname[BFNAMELEN + 1];
        char *ptr;

        if(path == NULL || path[0]==0) return "";

        if (strncmp(path, "boards/", 7)) return "";
        strlcpy(bname, &path[7], sizeof(bname));
        ptr = strtok(bname, " /\r\n\t");
        if (ptr == NULL) return "";
        return ptr;
}

int announce_search() {
	init_all();
	
	hs_init(1);
	hs_setfile("search_0an.ptn");
	hs_end();
	http_quit();
	return 0;
}

int m_announce_stat(){
#define MAX_ANN_COUNT	100
	char board[BFNAMELEN + 1], path[STRLEN];
	char id[MAX_ANN_COUNT][IDLEN + 2];
	char buf[10][256];
	struct boardheader *board_header;
	struct anntrace x;
	struct tm *ltime;
	int total[MAX_ANN_COUNT], id_num, i;
	int year1, month1, year2, month2;
	FILE *fp;
		
	init_all();
	if(!HAS_PERM(PERM_OBOARDS))
		http_fatal("错误的页面");
	strsncpy(board, getparm("board"), BFNAMELEN);
	if(board[0] == 0){
		hs_init(1);
		hs_setfile("bbsannstat.ptn");
		hs_end();
		http_quit();
	}
	board_header=getbcache(board);
	if(board_header==0) http_fatal("错误的讨论区");
	year1 = atoi(getparm("year1"));
	month1 = atoi(getparm("month1"));
	year2 = atoi(getparm("year2"));
	month2 = atoi(getparm("month2"));
	if(year1 == 0 || month1 == 0)
		http_fatal("请输入时间");
	if(month1 > 12) month1 = 12;
	if(month2 > 12)	month2 = 12;
	if(year2 == 0 || month2 == 0){
		year2 = year1;
		month2 = month1;
	}
	snprintf(path, sizeof(path), "0Announce/boards/%s/.trace", board_header->filename);
	if( (fp = fopen(path, "r")) == NULL)
		http_fatal("can't open file");
	
	id_num = 0;
	/*char *ptr;
	ptr = strtok(board_header->BM, " ");
	while(ptr != NULL){
		strcpy(id[i], x.executive);
		total[i] = 0;
		id_num++;
		ptr = strtok(NULL, " ");
	}*/
	while(!feof(fp)){
		if(fread(&x, sizeof(x), 1, fp) <= 0)
			break;
		ltime = localtime(&x.otime);
		
		if((ltime->tm_year + 1900 > year2)
		|| (ltime->tm_year + 1900 == year2 && ltime->tm_mon + 1 > month2))
			break;
		if( ((ltime->tm_year + 1900 > year1)
		|| (ltime->tm_year + 1900 == year1 && ltime->tm_mon + 1 >= month1)
		) && ((ltime->tm_year + 1900 < year2)
		 	|| (ltime->tm_year + 1900 == year2 && ltime->tm_mon + 1 <= month2)
		)){
			if(strstr(x.location, "留言"))
				continue;
			for(i = 0; i < id_num && i < MAX_ANN_COUNT; i++) {
				if(strcmp(id[i], x.executive) == 0) {
					total[i]++;
					break;
				}
			}
			if(i == id_num) {
				strcpy(id[i], x.executive);
				total[i] = 1;
				id_num++;
			}
		}
	}
	if(id_num ==0)
		http_fatal("精华区没有任何操作记录");
	hs_init(20);
	hs_setfile("m_annstat.ptn");
	hs_assign("BOARD", board_header->filename);
	
	hs_setloop("mainform");
	for(i = 0; i < id_num; i++) {
		sprintf(buf[0], "%d", i);
		hs_assign("INDEX", buf[0]);
		hs_assign("ID", id[i]);
		if(year1 == year2 && month1 == month2)
			sprintf(buf[1], "%d年%d月", year1, month1);
		else
			sprintf(buf[1], "从 %d年%d月 至 %d年%d月", year1, month1, year2, month2);
		hs_assign("DATE", buf[1]);
		sprintf(buf[2], "%d", total[i]);
		hs_assign("COUNT", buf[2]);
		if(strstr(board_header->BM, id[i]))
			hs_assign("STATUS", "<font color='#0000ff'>现任版主</font>");
		else
			hs_assign("STATUS", "&nbsp;");
		hs_doloop("mainform");
	}
	hs_end();
	http_quit();
	return 0;
}
