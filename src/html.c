#include "webbs.h"

static int hh_color=37;

int
get_color(){
	return hh_color;
}

int
set_color(int clr){
	hh_color = clr;
	return 0;
}

/* ------------ modified by wood to convert html tab -------------- */
/* assert ( the converted string size < 1024 ) */
char *
nohtml (char *s)
{
//      char *buf=calloc(strlen(s)+1, 1);
	static char buf[1024];
        int i=0; //, cnt;
//        char *buf;
/*
	while(s[i]) {
                if(s[i]=='<' || s[i]=='>')  cnt++;
                i++;
        }
*/
        i=0;
	/* Henry: this cause a memory leak */
//        buf=calloc(strlen(s)+cnt*3+1, 1);
        while(s[0] && i<1000) {
                if(s[0]=='<') {
                        strcpy(buf+i, "&lt;");
                        i+=4;
                } else if(s[0]=='>') {   
                        strcpy(buf+i, "&gt;");
                        i+=4;
		} else if(s[0]=='"') {
			strcpy(buf+i, "&quot;");
			i+=6;
    } else if(s[0]=='&') {
      strcpy(buf+i, "&amp;");
      i+=5;
		} else {
                        buf[i]=s[0];
                        i++;
                }
                s++;
        }
        buf[i]=0;
//      if (buf[0]==' ') strcat(buf, "&nbsp");
        return buf;
}

char *
noansi (s)
char *s; {
        static char buf[1024];
        int i=0, mode=0;
        while(s[0] && i<1023) {
                if(mode==0) {
                        if(s[0]==27) {
                                mode=1;
                        } else {
                                buf[i]=s[0];
                                i++;
                        }
                } else {
                        if(!strchr(";[0123456789", s[0])) mode=0;
                }
                s++;
        }
        buf[i]=0;
        return buf;
}

char *
strright (s, len)
char *s;
int len; {
        int l=strlen(s);
        if(len<=0) return "";
        if(len>=l) return s;
        return s+(l-len);
}

char *
ltrim (s)
char *s; {
        char *s2=s;
        if(s[0]==0) return s;
        while(s2[0] && strchr(" \t\r\n", s2[0])) s2++;
        return s2;  
}       

char *
rtrim (s)
char *s; { 
        static char t[1024], *t2;
        if(s[0]==0) return s;
        strsncpy(t, s, 1024);
        t2=t+strlen(s)-1;
        while(strchr(" \t\r\n", t2[0]) && t2>t) t2--;
        t2[1]=0;
        return t;     
}

void 
http_fatal (char *fmt, ...) {
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
//        vsprintf(buf, vsnp2vsp(fmt), ap);
        vsprintf(buf, fmt, ap);
        va_end(ap);
        buf[1023]=0;
	
	printf("<html>"
		"<head>"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />"
		"<title>中山大学逸仙时空BBS</title>"
		"<link href=\"templates/global.css\" rel=\"stylesheet\" type=\"text/css\" />"
		"<style type=\"text/css\">"
		"  body{"
		"    margin-right:16px;"
				 " }"
		"</style>"
		"</head>"

		"<body>"
		"<form name=\"form1\" method=\"post\" action=\"\">"
		"<div id=\"head\">"
		"<div id=\"location\">"
		"<p><img src=\"images/location01.gif\" align=\"absmiddle\"/><a href=\"bbssec\">%s</a></p>"
		"</div>"
		"</div>"
		"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"table\">"
		"<tr>" 
		"<td colspan=\"2\" class=\"tb_head\"><img src=\"images/table_ul_warning.gif\" alt=\"\" width=\"37\" height=\"30\" align=\"absmiddle\" class=\"tb_ul\" style=\"float:left\" /> "
		"<div style=\"margin-top:8px\"></div></td>"
		"<td width=\"19\" align=\"right\" valign=\"top\" class=\"tb_r\"><img src=\"images/table_ur.gif\"/></td>"
		"</tr>"
		"<tr>" 
		"<td class=\"tb_l\">&nbsp; </td>"
		"<td>"
		"<ul class=\"search\">"
		"<li>%s</li>"
		"<li><a href=javascript:history.go(-1)><img src=\"images/back.gif\" border=\"0\"/></a></li>"
		"</ul>"		
		"</td>"
		"<td class=\"tb_r\">&nbsp;</td>"
		"</tr>"
		"<tr class=\"tb_bottom\">"
		"<td width=\"25\"><img src=\"images/table_bl.gif\" alt=\"\"/></td>"
		"<td width=\"1063\"></td>"
		"<td align=\"right\"><img src=\"images/table_br.gif\" alt=\"\"/></td>"
		"</tr>"
		"</table>"
		"</form>"
		"</body>"
		"</html>"
	, BBSNAME, buf);
	http_term();
}

int 
hsprintf (char *s, char *fmt, ...) {
        char buf[1024], ansibuf[80], buf2[80];
        char *tmp;
        int c, bold, m, i, l, len;
        va_list ap;
        va_start(ap, fmt);
        vsprintf(buf, (fmt), ap);
        va_end(ap);
        s[0]=0;
        l=strlen(buf);
        len=0;
        bold=0;
        for(i=0; i<l; i++) {
                c=buf[i];   
                if(c=='&') {
                        strnncpy(s, &len, "&amp;");
                } else if(c=='<') {
                        strnncpy(s, &len, "&lt;");
                } else if(c=='>') {
                        strnncpy(s, &len, "&gt;");
		} else if(c=='\n'){
			strnncpy(s, &len, "<br/>");
                } else if(c==27) {
                        if(buf[i+1]!='[') continue;
                        for(m=i+2; m<l && m<i+24; m++)
                                if(strchr("0123456789;", buf[m])==0) break;
                        strsncpy(ansibuf, &buf[i+2], m-(i+2)+1);
                        i=m;
                        if(buf[i]!='m') continue;
                        if(strlen(ansibuf)==0) {
                                bold=0;
                                strnncpy(s, &len, "<font class=c37>");
                        }
                        tmp=strtok(ansibuf, ";");
                        while(tmp) {
                                c=atoi(tmp);
                                tmp=strtok(0, ";");
                                if(c==0) {
                                        strnncpy(s, &len, "<font class=c37>");
                                        bold=0;
                                }
                                if(c>=30 && c<=37) {
                                        if(bold==1) sprintf(buf2, "<font class=d%d>", c);
                                        if(bold==0) sprintf(buf2, "<font class=c%d>", c);
                                        strnncpy(s, &len, buf2);
                                }
                        }
                } else {
                        s[len]=c;
                        len++;
                }
        }
        s[len]=0;
	return len;
}

int hprintf (char *fmt, ...) {
        char buf[8096], buf2[1024];
        va_list ap;
        va_start(ap, fmt);
        vsprintf(buf2, (fmt), ap);
        va_end(ap);
        hsprintf(buf, "%s", buf2);
        printf("%s", buf);
	return strlen(buf);
}

char *utf_encoder(char *s) {
	return s;
}

char *ansi_ctrl(char *s) {
	char ansibuf[20], *ptr;
	int len, c, i;

	if (*(s + 1) != '[') {
		printf("%c", *s);
		return s + 1;
	}

	for (i = 2; s[i]; i++) {
		if (strchr("0123456789;", s[i]) == 0) break;
	}

	len = i;
	if (len > 16 || s[i] != 'm') return s + len + 1;
	strlcpy(ansibuf, s + 2, len);
	ptr = strtok(ansibuf, ";");
	while (ptr) {
		c = atoi(ptr);
		if(!strcmp(ptr, "m")){
		//if (c == 0) {
			set_color(37);
			printf("</font><font class=c37>");
		} else if (c >= 30 && c <= 37) {
			set_color(c);
			printf("</font><font class=c%d>", c);
			//printf("</font><font class=%c%d>", bold?'d':'c', c);
		}
		ptr = strtok(NULL, ";");
	}
	return s + len + 1;
}

int hhprintf (char *fmt, ...) {
        char buf[1024], *s, ch;
        int len = 0;
        va_list ap;
        va_start(ap, fmt);
        vsprintf(buf, (fmt), ap);
        va_end(ap);
        buf[1023]=0;

        s=buf;
        int quoted = 0;
        if(!strncmp(s, ": ", 2)) quoted = 1;
        while(*s) {
		if (len >= 100) {
			printf("\n");
			len = 0;
		}
                if(!strncasecmp(s, "http://", 7) || !strncasecmp(s, "mailto:", 7) || !strncasecmp(s, "ftp://", 6)) {
                        char *tmp;
			tmp = s;
			while (*tmp && *tmp > 0 && strchr("\'\" \r\t)(,;\n\033", *tmp) == NULL) tmp++;
			ch = *tmp;
			*tmp = '\0';
			
			printf("<a target=_blank href='%s'>", utf_encoder(s));
			if ((!strncasecmp(tmp - 4, ".jpg", 4) || !strncasecmp(tmp - 5, ".jpeg", 5) ||
			    !strncasecmp(tmp - 4, ".gif", 4) || !strncasecmp(tmp - 4, ".bmp", 4) ||
			    !strncasecmp(tmp - 4, ".png", 4)) && !quoted)  {
				printf("<img src='%s'  onLoad=\"javascript:if(this.width>570)this.width=570;\" onMouseover=\"javascript:if(this.width>570)this.width=570;\"></a>\n", utf_encoder(s));
				len = 0;
			} else {
				while (*s) {
					while (*s && len < 100) {
						printf("%c", *s);
						len ++;
						s ++;
					}
					if (*s == 0) break;
					printf("\n");
					len = 0;
				}
				printf("</a>");
			}
			*tmp = ch;
			s = tmp;
		} else {
			len ++;
			/*babydragon: 
			 * 如果是汉字, 则连续输出2个字节
			 * 如果是半个汉字, 则忽略不输出
			 */
			if(*s < 0){
				if(*(s+1)){
					printf("%c%c", *s, *(s+1));
					s += 2;
					len++;
				}else{
					s++;
				}
			}else if(*s == '<'){
				printf("&lt;");
				s++;
			}else if(*s == '>'){
				printf("&gt;");
				s++;
			}else if(*s == '&'){
				printf("&amp;");
				s++;
			}else if(*s == ' '){
				printf("&nbsp;");
				s++;
			}else if(*s == 033){
				s = ansi_ctrl(s);
				len --;
			}else if(*s == '\n'){
				printf("%c", *s);
				s++;
				len = 0;
			}else{
				printf("%c", *s);
				s++;
			}
		}
        }
	return len;
}

char *
void1 (s)
unsigned char *s; {
        int i;
        int flag=0;
        for(i=0; s[i]; i++) {
                if(flag==0) {
                        if(s[i]>=128) flag=1;
                        continue;
                }
                flag=0;
                if(s[i]<32) s[i-1]=32;
        }
        if(flag) s[strlen(s)-1]=0;
        return s;
}

char *
userid_str (s)
char *s; {
        static char buf[512];
        char buf2[256], tmp[256], *ptr, *ptr2;
        strsncpy(tmp, s, 255);
        buf[0]=0;
        ptr=strtok(tmp, " ,();\r\n\t");
        while(ptr && strlen(buf)<400) {
		ptr2 = strchr(ptr, '.');
                if(ptr2) {
                        ptr2[1]=0;
                        strcat(buf, ptr);
                        strcat(buf, " ");
                } else {
                        ptr=nohtml(ptr);   
                        sprintf(buf2, "<a href=bbsqry?userid=%s>%s</a> ", ptr, ptr);
                        strcat(buf, buf2);
                }
                ptr=strtok(0, " ,();\r\n\t");
        }
        return buf;
}

char *
nbsp (s)
const char *s; {
        int i, spacenum;
        static char s1[1024];  
        char *ptr1, *ptr2;
                        
        if (strlen(s) >= 1024) return "";
        spacenum = 0;
        for (i = 0; i < strlen(s); i++)
                if (s[i] == ' ') spacenum ++;
        if (strlen(s) + spacenum * 4 >= 1024) return "";
        for (ptr1 = (char *)s, ptr2 = (char *) &s1[0]; *ptr1; ptr1++, ptr2++) {
                if (*ptr1 == ' ') {
                        strncpy(ptr2, "&nbsp;", 6);
                        ptr2 += 5;
                } else {
			*ptr2 = *ptr1;
		}
        }
	*ptr2 = 0;
        return s1;
}
        
void 
form_header (href, width)
char *href[];
int *width; {
	int i, flag=0;

	for (i=0; href[i] != NULL; i++) 
		if (width[i] == 0) flag=1;

	printf ("<!--表头-->");
	printf ("<center>");
	printf ("<TABLE border=1 cellPadding=0 cellSpacing=0 %s align=center "
		"bordercolor=#0099cc><TBODY><TR align=middle>"
		, flag?"width=100%%":"");

	for (i=0; href[i] != NULL; i++) 
		printf("<TD class=body width=%s height=27>%s</TD>",
			(width[i]>0)?inttostr(width[i]):"*", href[i]);

	printf("</TR></TBODY></TABLE>");
}

void 
form_content (href, width, highlight)
char *href[];
int *width;
int highlight; {
	int i, flag=0;


	for (i=0; href[i]; i++) 
		if (!width[i]) flag=1;

	printf("<TABLE border=1 cellPadding=0 cellSpacing=0 width=%s align=center "
		"bordercolor=#0099cc><TBODY><TR align=middle>"
	, flag?"100%%":"");

	for (i=0; href[i]; i++) {
		if (i==highlight) 
			printf("<TD align=left bgcolor=#f2f8ff width=%d "
				"onmouseover=javascript:this.bgColor='#e8f4ff' "
				"onmouseout=javascript:this.bgColor='#f2f8ff'>",
			width[i]);
		else printf("<TD class=body%d width=%d height=27>",
			abs(highlight-i+1) % 2 + 1, width[i]);

		printf("%s</TD>", href[i]);
	}

	printf("</TR></TBODY></TABLE>");
}

void 
form_foot (int start, int total, int lines, char *fmt, ...) {
	char buf[255];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, (fmt), ap);
	va_end(ap);

	printf("<table border=0 cellpadding=0 cellspacing=3 width=100%% align=center>");
	printf("<form name=form1 action=%s method=post>\n", buf);
	printf("<tr><td valign=middle><font color=#000000>");
	printf("页次：<b>%d</b>/<b>%d</b>页", start/lines+1, total/lines+1);
	printf(" 每页<b>%d</b> 总数<b>%d</b></font></td>", lines, total);
	printf("<td valign=middle><div align=right ><font color=#000000><p>分页：<font face=webdings>");
	if (strchr(buf, '?'))
		strcat(buf, "&");
	else
		strcat(buf, "?");
	strcat(buf, "start=");
	if (start>1) {
        	printf("<font color=000000><a href=%s1>9</a>", buf);
	        printf(" <a href=%s%d>3</a>", buf, start-lines);
	} else {
        	printf("<font color=ff0000>9 3");
	}
	printf("</font> ");
	if (start<=total-lines) {
        	printf("<font color=000000><a href=%s%d>4</a>", buf, start+lines);
        	printf(" <a href=%s%d>:</a>", buf, total);
	} else {
        	printf("<font color=ff0000>4 :");
	}
	printf("</font>");
	printf("</font>   转到:第<input type=text name=start size=3 ");
	printf("maxlength=10  value='1'>项<input type=submit value=Go>");
	printf("</p></font></div></td></tr></form></table>\n");
} 

void 
postheader () {
printf (
	"<table cellspacing=0 border=0 width=100%% bgcolor=#0099cc "
	"align=center><tr><td height=1></td></tr></table>"
	"<table bordercolor=#0099cc cellspacing=0 cellpadding=0 width=100%% "
	"align=center border=1><tbody><tr align=middle>"
	"<td class=body vAlign=center height=5></td></tr></tbody></table>"
        "<table bordercolor=#0099cc cellspacing=0 cellpadding=0 width=100%% "
        "align=center border=1>"
        "<tbody><tr align=middle><td class=body2 vAlign=center height=25>"
        "发表新文章"
        "</td></tr></tbody></table>");
}

void 
form_title (x)
struct title_t *x; {
	struct boardheader *data;
	int i, j;
	int flag=0;
	char format[3][STRLEN];
	char href[3][STRLEN];
	char *space="&nbsp&nbsp";
	rightclick();
	printf("<TABLE border=0 width=100%% align=center><TBODY><TR>");
	printf("<TD vAlign=top width=60%%>");
	printf("<a href=%s target=_top border=0>%s</a></TD>", BBSHHOST, BANNER);
	printf("<TD valign=middle align=top>");
	strcpy(href[0], "bbssec");
	sprintf(format[0], "【%s】", BBSNAME);

	data = getbcache(x->board);
	i = getsecnum(x->board);
	if (i != -1) {
		sprintf(href[1], "bbsboa?board=%d", i);
		sprintf(format[1], "【%s】", secname[i][0]);
		flag = 1;
		sprintf(href[2], "bbsdoc?board=%s", x->board);
		sprintf(format[2], "【%s】", data->title+11);
		if (!flag) {
			strcpy(format[1], format[2]);
			x->depth--;
			x->num=0; x->BM=0;
		}
	} else if (atoi(x->board) || !strcmp(x->board, "0")) {
		sprintf(href[1], "bbsboa?board=%s", x->board);
		sprintf(format[1], "【%s】", secname[atoi(x->board)][0]);
	} else {
		sprintf(format[1], "【%s】", x->board);
	}
	for (i=0; i<3; i++) {
		for (j=0; j<i; j++) printf("%s", space);
		if (i==x->depth) {
			printf("<img src=/folder2.gif>%s", space);
			printf("<font color=ff0000>%s</font>", format[i]);
			break;
		} else {
			printf("<img src=/folder.gif>%s", space);
			printf("<a href=%s>%s</a><br>", href[i], format[i]);
		}
	}
	printf("</TD></TR></TBODY></TABLE>");
	printf("<table cellpadding=0 cellspacing=0 border=0 width=100%% "
		"align=center valign=middle><tr>");
	if (x->BM) {
		if ((userid_str(data->BM))[0])
		        printf("<td align=right>版主: %s</td></tr></table>",
               			userid_str(data->BM));
		else printf("<td align=right>诚征版主中</td></tr></table>");
	}
	printf("<table cellspacing=0 border=0 width=100%% bgcolor=#0099cc "
		"align=center><tr><td height=1></td></tr></table>");
	if (x->BM) {
		printf("<TABLE cellSpacing=0 cellPadding=0 width=100%% border=1 "
			"align=center bordercolor=#0099cc><TBODY><tr><td>"
		"<table width=100%% cellSpacing=0 cellPadding=3>"
		"<tr bgcolor=#f2f8ff>");
		printf("<td>\n");
		printf("<a href=bbspst?board=%s>", x->board);
		printf("<img src=/postnew.gif></a>");
		if (HAS_PERM(PERM_POST) && board_has_vote(x->board)) {
			printf("&nbsp&nbsp<a href=bbsvotedoc?board=%s>", x->board);
			printf("<img src=/votenew.gif></a>");
		}
		printf("</td>");
		printf("<td width=* align=right>%s", x->format[0]);
		for (i=1; i<x->num; i++)
			printf(" | %s", x->format[i]);
	}
	printf("</td></tr></table>");
	printf("</td></tr></TBODY></TABLE>");
}

void 
rightclick () {
	printf("<SCRIPT src='/rightmnu.js'></SCRIPT>\n");
}

void 
form_view_header () {
	printf(	"<TABLE class=tableborder1 style='TABLE-LAYOUT: "    
                "fixed; WORD-BREAK: break-all' cellSpacing=1 cellPadding=5 " 
                "align=center><TBODY><TR><TD class=body1 vAlign=top width=*>"
		);
}

void 
form_read_header (msg)
char *msg; {
printf( "<TABLE cellSpacing=0 cellPadding=0 width=100%% align=center border=0>"
	"  <TBODY>"
	"  <TR>"
	"    <TD vAlign=center width=1 bgColor=#0099cc height=24></TD>"
	"    <TD vAlign=center align=left width=* bgColor=#99ccff>"
	"      <TABLE cellSpacing=1 cellPadding=0 width=100%% border=0>"
	"        <TBODY>"
	"        <TR>"
	"          <TD vAlign=center align=left width=65%% bgColor=#99ccff><FONT "
	"            color=#000000>&nbsp;<B>* 主题</B>： %s"
	"          </TD></TR></TBODY></TABLE></TD>"
	"    <TD vAlign=center width=1 bgColor=#0099cc " 
	"height=24></TD></TR></TBODY></TABLE>", msg);
	form_view_header();
}

void 
mail_read_header (mailnum)
int mailnum; {
	FILE *fp;
	static char buf[256];
	struct fileheader x;
	char file[STRLEN], title[TITLELEN + 2];
	setmailfile(file, ".DIR");
	if (get_record(file, &x, sizeof(x), mailnum) == -1) {
		http_fatal("无法打开索引文件");
	}
	setmailfile(file, x.filename);
	fp = fopen(file, "r");
	if (fp == NULL) http_fatal("Can't read mail");
	fgets(buf, 256, fp);
	fgets(buf, 256, fp);
	strlcpy(title, (char *)&buf + 8, TITLELEN + 1);
	form_read_header(title);

	printf( " <a href=bbspstmail>"
		"<img src=/email.gif>发新信</a>"
		" <a href=bbspstmail?filenum=%d>"
		"<img src=/quote.gif>转寄</a>"
		" <a onclick='return confirm(\"你真的要删除这封信吗?\")"
		"' href=bbsdelmail?file=%s>"
        	"<img src=/del.gif>删除信件</a>"
		" <a href=bbspstmail?filenum=%d>"
        	"<img src=/replynow.gif>回信</a>"
		"<BR><HR class=tableborder1 SIZE=1><BLOCKQUOTE><pre>"
		, mailnum, x.filename, mailnum);
}

void 
article_read_header (board, x)
char *board;
struct fileheader *x; {
	char file[33], *title;
	strlcpy(file, x->filename, sizeof(file));
	title=x->title;
	if (strncmp(title, "Re: ", 4) == 0) title += 4;
	title[TITLELEN - 1]=0;

	printf("<TABLE cellSpacing=0 cellPadding=0 width=100%% "
		"align=center border=1 bordercolor=#0099cc><TBODY><TR>"
		"<TD vAlign=center align=left width=100%%>&nbsp; <A "
		"href=bbspst?board=%s><IMG alt=发表一个新主题 "
		"src=/postnew.gif></A>&nbsp;<A "
		"href=bbspst?board=%s&file=%s&userid=%s><IMG "
		"alt=回复主题 src=/reply.gif></A></TD>"
		"<TD vAlign=center align=right width=70%%>"
		"</TD></TR></TBODY></TABLE>",
		board, board, file, x->owner);
	form_read_header(title);
	printf( " <a href=bbsfwd?board=%s&file=%s>"
		"<img src=/email.gif>转寄/推荐</a>"
		" <a href=bbsccc?board=%s&file=%s>"
        	"<img src=/quote.gif>转贴</a>"
		" <a onclick='return confirm(\"删除的文章将无法恢复，"
		"你真的要删除本文吗?\")' href=bbsdel?board=%s&file=%s>"
        	"<img src=/del.gif>删除文章</a>"
		" <a href=bbsedit?board=%s&file=%s>"
		"<img src=/edit.gif>修改文章</a>"
		" <a href=bbspst?board=%s&file=%s&userid=%s>"
        	"<img src=/replynow.gif>回文章</a>"
		" <a href=bbstcon?board=%s&file=M.%d.A>"
        	"<font style='color:#f00000'><img src=/message.gif>同主题阅读</font></a>"
		"<BR><HR class=tableborder1 SIZE=1><BLOCKQUOTE><pre>"
		, board, file, board, file, board, file
		, board, file, board, file, x->owner
		, board, x->id);
}

void 
form_view_foot () {
	printf("</pre></TD></TR></TBODY></TABLE>");
}	

void 
form_read_foot () {
	printf("</BLOCKQUOTE>");
	form_view_foot();
}

void 
form_post_header (board, title, articleid)
char *board;
char *title;
int articleid; {
	char *pos;

	printf("<FORM method=post action=bbssnd?board=%s", board);
#ifdef ATTACH_UPLOAD
	struct boardheader *bptr;
	bptr = getbcache(board);
	if (bptr && (bptr->flag & BRD_ATTACH)) { /* Henry: 检查版面附件标记 */
		printf(" enctype=\"multipart/form-data\"");
	}
#endif
	printf(">");
	while ((pos = strchr(title, '\'')) != NULL) {
		*pos = '\"';
	}
	printf("<TABLE class=tableborder1 cellSpacing=1 cellPadding=3 align=center>"
	"<TBODY><TR><TH align=left width=100%% colSpan=2 height=25>"
	"&nbsp;&nbsp;发表新帖子</TH></TR><TR><TD class=tablebody1 width=20%%>"
	"<B>主题标题</B> </TD><TD class=tablebody1 width=80%%>");

	printf("<INPUT maxLength=80 size=55 name=title value='%s'>", void1(title));
	printf("<INPUT type=hidden name=oldtitle value='%s'>", void1(title));
	if (articleid)
		printf("<INPUT type=hidden name=articleid value=%d>", articleid);

	printf("<FONT color=#ff0000><STRONG>*</STRONG></FONT>不得超过 25 个汉字" 
	"</TD></TR><TR><TD class=tablebody1 vAlign=top width=20%%>"
	"<b>发文注意事项: </b><br><br><font color='#006600'>"
	"发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。"
	"</font></TD><TD class=tablebody1 width=80%%>"
	"<textarea name=text rows=18 cols=80>\n\n");
}

void 
form_post_foot (board, title)
char *board;
char *title; {
	printf("</TEXTAREA></TD></TR><TR>"
	"<TD class=tablebody1 vAlign=top><FONT color=#ab0000 underline;}"
	"TEXT-DECORATION: #FFFFFF; COLOR: { A:hover #TableTitleLink } none;"
	"A:active A:visited, A:link,><B>选项</B></FONT></TD>"
	"<TD class=tablebody1 vAlign=center>使用签名档"
        "<input type=radio name=signature value=1 checked>1"
        "<input type=radio name=signature value=2>2"
        "<input type=radio name=signature value=3>3"
        "<input type=radio name=signature value=4>4"
        "<input type=radio name=signature value=5>5"
        "<input type=radio name=signature value=0>0"
        "[<a target=_balnk href=bbssig>查看签名档</a>] &nbsp;");

        if (anonyboard(board))
        {
                printf ("<input type=checkbox name=anonymous value=Y checked>匿名&nbsp;");
        }

        printf("<input type=checkbox name=exchange value='Y' %s>转信",
                isOutgoingBoard(board)?"checked":"");

        printf("</TD></TR><TR>");

#ifdef ATTACH_UPLOAD
	struct boardheader *bptr;

	bptr = getbcache(board);
	if (bptr && (bptr->flag & BRD_ATTACH)) {
		printf(
		"<TD class=tablebody1 vAlign=top><B>附件上传</B></TD>"
		"<TD class=tablebody1 vAlign=center>"
		"附件 <input type=file name=filename maxlength=40 size=8>"
		"</TD></TR><TR>");
	}
#endif

	if (!loginok) printf(
	"<TD class=tablebody1 vAlign=top><B>登陆选项</B></TD>"
	"<TD class=tablebody1 vAlign=center>"
	"帐号 <input type=text name=id maxlength=12 size=8>"
	"密码 <input type=password name=pw maxlength=40 size=8>"
	"</TD></TR><TR>");

	printf("<TD class=tablebody2 vAlign=center align=middle colSpan=2>"
	"<INPUT type=submit value='发 表' name=Submit> &nbsp;"
	"<INPUT type=reset value='清 除' name=Submit2></TD>");

	printf("<input type=hidden name=oldtitle value='%s'>", void1(title));

	printf("</TBODY></TABLE></FORM>");
}

int cmpfileheader(void *fh1, void *fh2) {
	return (!memcmp((struct fileheader *)fh1, (struct fileheader *)fh2, 
		sizeof(struct fileheader)));
}

void 
mail_post_header (x)
struct fileheader *x; {
	char title[TITLELEN+2];
	char *to_userid = NULL, userid[IDLEN+2];
	char mailfname[STRLEN];
	struct fileheader fhtemp;
	int n;

	setmailfile(mailfname, ".DIR");
	n = search_record(mailfname, &fhtemp, sizeof(fhtemp), cmpfileheader, x);

	if (x) {
		strlcpy(title, x->title, TITLELEN + 1);
		if (strncmp(x->title, "Re:",3))
			sprintf(title, "Re: %s", x->title);
		strlcpy(userid, nohtml(x->owner), IDLEN + 1);
		to_userid = strtok(userid, " (\n\r\t");
	}

	if (x)
		printf("<FORM method=post action=bbssndmail?userid=%s>", to_userid);
	else
		printf("<FORM method=post action=bbssndmail>");

	printf("<TABLE class=tableborder1 cellSpacing=1 cellPadding=3 align=center>"
	"<TBODY><TR><TH align=left width=100%% colSpan=2 height=25>"
	"&nbsp;&nbsp;寄语信鸽</TH></TR><TR><TD class=tablebody1 width=20%%>"
	"<B>信件标题</B> </TD><TD class=tablebody1 width=80%%>");

	printf("<INPUT type=hidden name=filenum value=\"%d\">", n);

	printf("<INPUT maxLength=55 size=55 name=title value=\"%s\">", 
		x?void1(title):"");

	printf("<FONT color=#ff0000><STRONG>*</STRONG></FONT>不得超过 50 个汉字" 
	"</TD></TR><TR><TD class=tablebody1 vAlign=top width=20%%><b>收信人</b><br>");

	if (!x) printf("<input type=text maxLength=15 size=15 name=userid>");
	else printf("%s", to_userid);
	printf("</TD><TD class=tablebody1 width=80%%>"
		"<textarea name=text rows=18 cols=80>\n\n");
}

void 
mail_post_foot () {
	printf("</TEXTAREA></TD></TR><TR>"
	"<TD class=tablebody1 vAlign=top><FONT color=#ab0000 underline;}"
	"TEXT-DECORATION: #FFFFFF; COLOR: { A:hover #TableTitleLink } none;"
	"A:active A:visited, A:link,><B>选项</B></FONT></TD>"
	"<TD class=tablebody1 vAlign=center>使用签名档"
        "<input type=radio name=signature value=1 checked>1"
        "<input type=radio name=signature value=2>2"
        "<input type=radio name=signature value=3>3"
        "<input type=radio name=signature value=4>4"
        "<input type=radio name=signature value=5>5"
        "<input type=radio name=signature value=0>0"
        "[<a target=_balnk href=bbssig>查看签名档</a>] &nbsp;");

	printf("<input type=checkbox name=backup>备份\n");

        printf("</TD></TR><TR>");

	printf("<TD class=tablebody2 vAlign=center align=middle colSpan=2>"
	"<INPUT type=submit value='发 送' name=Submit> &nbsp;"
	"<INPUT type=reset value='清 除' name=Submit2></TD>");

	printf("</TBODY></TABLE></FORM>");
}

char *
UTF8 (url)
char *url; {
	static char buf[1024];
	int i, j;
	j = 0;
	for (i=0; i<strlen(url); i++) {
		if (url[i] == '\n') {
			buf[j] = '%';	
			buf[j+1] = '0';
			buf[j+2] = 'A';
			j += 3;
		} else 	{
			buf[j] = url[i];
			j++;
		}
	}
	return buf;
}

void section_title() {
	printf("<TABLE border=0 width=100%% align=center><TBODY><TR>");
	printf("<TD vAlign=top width=60%%><a href=%s target=_top>%s</a></TD>", BBSHHOST, BANNER);
	printf("<TD valign=middle align=top><img src=/folder2.gif> ");
	printf("&nbsp;&nbsp;<font color=ff0000>【%s】</font>", BBSNAME);
	printf("</TD></TR></TBODY></TABLE>");
	rightclick();
}

void section_header() {
	printf("<table cellspacing=0 border=0 bgcolor=#0099cc width=378 align=center>");
	printf("<tr><td height=1></td></tr></table>");
	printf("<TABLE border=1 cellPadding=0 cellSpacing=0 align=center ");
	printf("bordercolor=#0099cc><TBODY><TR align=middle>");
	printf("<TD class=body width=32 height=27>区号</TD>");
	printf("<TD class=body width=64>类别</TD>");
	printf("<TD class=body width=90>描述</TD>");
	printf("<TD class=body width=32 height=27>区号</TD>");
	printf("<TD class=body width=64>类别</TD>");
	printf("<TD class=body width=90>描述</TD>");
	printf("</TR></TBODY></TABLE>");
}

void section_content(int i) {
	printf("<TABLE border=1 cellPadding=0 cellSpacing=0 align=center ");
	printf("bordercolor=#0099cc><TBODY><TR align=middle>");
	printf("<TD class=body1 width=32 height=27>%d</TD>", i);
	printf("<TD class=body2 width=64><a href=bbsboa?board=%d target=f3>%s</a></TD>",
		i, secname[i][0]);
	printf("<TD class=body1 width=90><a href=bbsboa?board=%d target=f3>%s</a></TD>",
		i, secname[i][1]);
	i = i + (SECNUM + 1) / 2;
	if (i < SECNUM) {
		printf("<TD class=body2 width=32 height=27>%d</TD>", i);
		printf("<TD class=body1 width=64><a href=bbsboa?board=%d target=f3>%s</a></TD>",
        		i, secname[i][0]);
		printf("<TD class=body2 width=90><a href=bbsboa?board=%d target=f3>%s</a></TD>",
        		i, secname[i][1]);
	} else {
		printf("<TD class=body2 width=32>&nbsp<TD class=body1 width=64>&nbsp<TD "
			"class=body2 width=90>&nbsp");
	}
	printf("</TR></TBODY></TABLE>");
}

char *get_content_type(char *extname) {
	if (!strcasecmp(extname, "txt")) return "text/html";
	else if (!strcasecmp(extname, "htm")) return "text/html";
	else if (!strcasecmp(extname, "html")) return "text/html";
	else if (!strcasecmp(extname, "jpg")) return "image/jpeg";
	else if (!strcasecmp(extname, "gif")) return "image/gif";
	else if (!strcasecmp(extname, "png")) return "image/x-png";
	else if (!strcasecmp(extname, "zip")) return "application/x-zip";
	return "application/octet-stream";
}
