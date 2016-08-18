#include "webbs.h"

int show_vote_list() {
	char *board, fname[STRLEN], controlfile[STRLEN];
	int total, i;
	char buf[7][STRLEN], secnum[STRLEN], secname[STRLEN];
	struct votebal currvote;
	struct boardheader *x1;
	
	init_all();
	if (!loginok) http_fatal("你还没有登陆");
	modify_user_mode(VOTING);
	board = getparm("board");
	if ((x1 = getbcache(board)) == NULL)
		http_fatal("错误的讨论区");
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if(!has_enter_perm(&currentuser, board)&&loginok) http_fatal("对不起，您目前被停止进入本板的权力!");
	if (!(total = board_has_vote(board))) http_fatal("没有投票举行");
	sprintf(controlfile, "vote/%s/control", board);

	hs_init(20);
	hs_setfile("bbsvotedoc.ptn");
	hs_assign("BOARD", board);
	sprintf(secnum, "%d", getsecnum(board));
	hs_assign("SECNUM", secnum);
	sprintf(secname, "%s", getsecname(board));
	hs_assign("SECNAME", secname);
	hs_assign("BOARDNAME", x1->title + 11);
	hs_setloop("mainform");
	for (i = 1; i <= total; i++) {
		get_record(controlfile, &currvote, sizeof(currvote), i);
		sprintf(buf[0], "%d", i);
		hs_assign("INDEX", buf[0]);
		sprintf(buf[1], "%s", userid_str(currvote.userid));
		hs_assign("USER", buf[1]);
		sprintf(buf[2], "%14.14s", getdatestring(currvote.opendate));
		hs_assign("DATE", buf[2]);
		sprintf(buf[3], "&nbsp;<a href=bbsvotecon?board=%s&num=%d>%s</a>", 
			board, i, currvote.title);
		hs_assign("TITLE", buf[3]);
		sprintf(buf[4], "%s", vote_type[currvote.type - 1]);
		hs_assign("TYPE", buf[4]);
		sprintf(buf[5], "%d", currvote.maxdays);
		hs_assign("DAY", buf[5]);
		sprintf(fname, "vote/%s/flag.%d", board, currvote.opendate);
		sprintf(buf[6], "%d", get_num_records(fname, sizeof(struct ballot)));
		hs_assign("NUM", buf[6]);
		hs_doloop("mainform");
	}
	hs_end();
	http_quit();
	return 0;
}

struct votebal currvote;

int
cmpvuid(void *userid, void *uv)
{
        return !strcmp((char *)userid, ((struct ballot *)uv)->uid);
}

void multivote(struct ballot *uv) {
	int i;
	unsigned int bits;

	bits = uv->voted;
	for (i = 0; i < currvote.totalitems; i++) {
		if (currvote.type == VOTE_MULTI) {
			printf("<input type=checkbox class=checkbox name=item%d value=item%d %s>%s",
				i, i, (bits >> i) & 1?"checked":"" ,currvote.items[i]);
		} else {
			printf("<input type=radio class=checkbox name=vote value=%d %s>%s",
				i, (bits >> i) & 1?"checked":"", currvote.items[i]);
		}
		printf("<br>");		
	}
}

void valuevote(struct ballot *uv) {
	char buf[10];
	printf("此次作答的值不能超过 %d<br>", currvote.maxtkt);
	if (uv->voted != 0)
		sprintf(buf, "%d", uv->voted);
	else
		memset(buf, 0, sizeof (buf));
	printf("你的答案: <input type=text name=vote value=%s size=10><br>", buf);
}

void askvote(struct ballot *uv) {
	int i;
	printf("<textarea name=comment rows=3 cols=80>");
	for (i = 0; i < 3; i ++) {
		if (uv->msg[i][0] == '\0') break;
		printf("%s\n", uv->msg[i]);
	}
	printf("</textarea>");
}

int do_vote() {
	char fname[STRLEN], *board, buf[512];
	int voted_flag;
	int num, pos;
	struct ballot uservote;
	FILE *fp;
	time_t date;
	struct boardheader *x1;
	
	init_all();
	if (!loginok) http_fatal("你还没有登陆");
	board = getparm("board");
	if ((x1 = getbcache(board)) == NULL)
		http_fatal("无效讨论区");
	num = atoi(getparm("num"));
	setvotefile(fname, board, "control");
	get_record(fname, &currvote, sizeof(struct votebal), num);
	if (currentuser.firstlogin > currvote.opendate) {
		http_fatal("对不起, 本投票在您帐号申请之前开启，您不能投票");
	} else if (!HAS_PERM(currvote.level & ~(LISTMASK | VOTEMASK))) {
		http_fatal("对不起，您目前尚无权在本票箱投票");
	} else if (currvote.level & LISTMASK) {
		setvotefile(fname, board, "vote.list");
		if (!file_exist(fname))
			http_fatal("对不起，本票箱需要设定好投票名册方可进行投票");
		else if (!seek_in_file(fname, currentuser.userid))
			http_fatal("对不起, 投票名册上找不到您的大名");
	} else if (currvote.level & VOTEMASK) {
		if (currentuser.numlogins < currvote.x_logins
		    || currentuser.numposts < currvote.x_posts
		    || currentuser.stay < currvote.x_stay * 3600
		    || currentuser.firstlogin >
		    currvote.opendate - currvote.x_live * 86400)
			http_fatal("对不起，您目前尚不够资格在本票箱投票");
	}
	sprintf(fname, "vote/%s/flag.%d", board, currvote.opendate);
	if ((pos = search_record(fname, &uservote, sizeof (uservote),
				cmpvuid, currentuser.userid)) <= 0) {
		(void) memset(&uservote, 0, sizeof (uservote)); 
		voted_flag = NA;
	} else {
		voted_flag = YEA;
	}
	strcpy(uservote.uid, currentuser.userid);
	sprintf(fname, "vote/%s/desc.%d", board, currvote.opendate);
	fp = fopen(fname, "r");
	if (fp == NULL) http_fatal("投票描述文件错误");
	date = currvote.opendate + currvote.maxdays * 86400;
	
	printf("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>"
		"<html>"
		"<head>"
		"<meta http-equiv='Content-Type' content='text/html; charset=gb2312' />"
		"<title>%s</title>"
		"<link href='templates/global.css' rel='stylesheet' type='text/css' />"
		"<style type='text/css'>"
		"body{        margin-right:16px;	  }"
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
	printf("<form action='bbsvotesnd' name='form1'>"
		"<table border='0' cellpadding='0' cellspacing='0' class='table'>"
		"<tr>"
		"<td colspan='2' class='tb_head'>"
		"<img src='images/table_ul06.gif' alt='' align='absmiddle' class='tb_ul' style='float:left' />" 
		"<div style='margin-top:8px'>参与投票</div> </td>"
		"<td align='right' valign='top' class='tb_head'><img src='images/table_ur.gif' alt=''/></td>"
		"</tr>"
		"<tr>"
		"<td width='100' height='29' class='td4'>投票主题:</td>"
		"<td colspan='2' class='td7'>%s</td>"
		"</tr>"
		"<tr>"
		"<td class='td4'>投票类型:</td>"
		"<td colspan='2' class='td7'>%s</td>"
		"</tr>"
		"<tr>"
		"<td class='td4'>结束时间:</td>"
		"<td colspan='2' class='td7'>%s</td>"
		"</tr>",
		currvote.title, vote_type[currvote.type - 1], getdatestring(date));
	printf("<tr>"
 		"<td class='td4'>"
		"<ul class='post_content'>"
		"<li>题目描述:</li><br/><li>");
	while (1) {
		if (!fgets(buf, 512, fp)) break;
		hhprintf("%s", void1(buf));
	}
	fclose(fp);
	printf("</li>"
		"<li class='notes'>* 请在右边填写您的选项。</li>"
		"</ul>"
		"</td>"
		"<td colspan='2' class='td7'>"
		"<input type=hidden name=board value=%s>"
		"<input type=hidden name=num value=%d>"
		"<ul class='post_content2' id='right_wrong'>"
		"<li>选项如下:</li><br/>",
		board, num);
	switch (currvote.type) {
	case VOTE_SINGLE:
	case VOTE_MULTI:
	case VOTE_YN:
		printf("\n");
		multivote(&uservote);
		break;
	case VOTE_VALUE:
		printf("\n");
		valuevote(&uservote);
		break;

	case VOTE_ASKING:
		askvote(&uservote);
		break;
	}
	if (currvote.type != VOTE_ASKING) {
		printf("<li>\n");
	  	printf("请填入您的宝贵意见(三行): <br/>");
		askvote(&uservote);
		printf("</li>\n");
	}
	printf("</td>"
		"</tr>"
		"<tr>" 
		"<td height='50' class='tb_head_left'>&nbsp;</td>"
		"<td colspan='2' class='tb_head_right'>"
		"<input type='image' src='images/post.gif' alt='发表' hspace='2px' class='confirm'/>"
		"<a href='javascript:form1.reset()'><img src='images/clear.gif' alt='重填' border='0'/></a></td>"
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

int post_vote() {
	char fname[STRLEN], *board, buf[512], *ptr;
	int voted_flag;
	int num, pos, item, i;
	unsigned int bits = 0;
	struct ballot uservote, tmpbal;

	init_all();
	if (!loginok) http_fatal("你还没有登陆");
	board = getparm("board");
	num = atoi(getparm("num"));
	sprintf(fname, "vote/%s/control", board);
	get_record(fname, &currvote, sizeof(struct votebal), num);
	if (currentuser.firstlogin > currvote.opendate) {
		http_fatal("对不起, 本投票在您帐号申请之前开启，您不能投票");
	} else if (!HAS_PERM(currvote.level & ~(LISTMASK | VOTEMASK))) {
		http_fatal("对不起，您目前尚无权在本票箱投票");
	} else if (currvote.level & LISTMASK) {
//		char listfilename[STRLEN];

	} else if (currvote.level & VOTEMASK) {
		if (currentuser.numlogins < currvote.x_logins
		    || currentuser.numposts < currvote.x_posts
		    || currentuser.stay < currvote.x_stay * 3600
		    || currentuser.firstlogin >
		    currvote.opendate - currvote.x_live * 86400)
			http_fatal("对不起，您目前尚不够资格在本票箱投票");
	}
	sprintf(fname, "vote/%s/flag.%d", board, currvote.opendate);
	if ((pos = search_record(fname, &uservote, sizeof (uservote),
				cmpvuid, currentuser.userid)) <= 0) {
		(void) memset(&uservote, 0, sizeof (uservote)); 
		voted_flag = NA;
	} else {
		voted_flag = YEA;
	}
	strcpy(uservote.uid, currentuser.userid);
	switch (currvote.type) {
	case VOTE_SINGLE:
	case VOTE_YN:
		item = atoi(getparm("vote"));
		if (item < 0 || item >= currvote.totalitems)
			http_fatal("非法选项");
		bits = (1 << item);
		break;
	case VOTE_MULTI:
		bits = 0;
		for (i = 0; i < currvote.totalitems; i++) {
			sprintf(buf, "item%d", i);
			if (!strncmp(getparm(buf), buf, 5)) {
				bits ^= (1 << i);
			}
		}
		break;
	case VOTE_VALUE:
		bits = abs(atoi(getparm("vote")));
		if (bits > currvote.maxtkt)
			http_fatal("无效的数值");
		break;
	case VOTE_ASKING:
		i = 0;
		strlcpy(buf, getparm("comment"), 512);
		ptr = strtok(getparm("comment"), "\n\r\0");
		while (ptr != NULL && i < 3) {
			strcpy(uservote.msg[i], ptr);
			do {
				ptr = strtok(NULL, "\n\r\0");
			} while (ptr != NULL && ptr[0] == 0);
			i++;
		}
		break;
	}
	if (currvote.type != VOTE_ASKING && uservote.voted == bits) {
		http_fatal("保留【%s】原来投票\n", currvote.title);
		//sprintf(buf, "bbsdoc?board=%s", board);
		//refreshto(buf, 3);
		http_term();
	}
	uservote.voted = bits;
	strlcpy(uservote.votehost, currentuser.lasthost, 16);
	uservote.votetime = time(NULL);
	sprintf(fname, "vote/%s/flag.%d", board, currvote.opendate);
	pos = search_record(fname, &tmpbal, sizeof (tmpbal),
		cmpvuid, currentuser.userid);
	if (pos) {
		if (substitute_record(fname, &uservote, sizeof (uservote), pos) == -1)
			http_fatal("投票失败");
	} else if (append_record(fname, &uservote, sizeof (uservote)) == -1) {
		http_fatal("投票失败! 请通知站长参加那一个选项投票");
	}
	http_fatal("已经帮您投入票箱中...");
	//sprintf(buf, "bbsdoc?board=%s", board);
	//refreshto(buf, 3);
	http_quit();
	return 0;
}

int board_has_vote(char *board) {
	char fname[STRLEN];
	int total;

	sprintf(fname, "vote/%s/control", board);
	total = file_size(fname);
	return (total / sizeof(struct votebal));
}
