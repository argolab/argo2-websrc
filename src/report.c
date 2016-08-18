#include "webbs.h"

const char *trace_file = "wwwlog/trace";

/* The following code is adopted from PukeBBS telnet version */

#ifdef MSGQUEUE

static int msqid = -1;
static struct bbsmsg msg = { mtype: BBSMSGTYPE };

void
do_report(int msgtype, char *str)
{
	if (msqid == -1) {
		msqid = msgget(MSQKEY, 0644);
	}
        msg.msgtype = msgtype;
        strlcpy(msg.message, str, sizeof(msg.message));
        msgsnd(msqid, &msg, sizeof(msg), 0);
}

void
report(char *fmt, ...)
{
        va_list ap;
        char buf[210];
        time_t now;

	if (msqid == -1) {
		msqid = msgget(MSQKEY, 0644);
	}

        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);

        now = time(NULL);
        msg.msgtype = LOG_TRACE;
        sprintf(msg.message, "%s %24.24s %s\n", currentuser.userid, ctime(&now), buf);
        msgsnd(msqid, &msg, sizeof(msg), 0);
}

#else

void
do_report(char *filename, char *str)
{
        char buf[512];
        time_t now;

        now = time(NULL);
        sprintf(buf, "%s %24.24s %s\n", currentuser.userid, ctime(&now), str);
        file_append(filename, buf);
}

void
report(char *fmt, ...)
{
        va_list ap;
        char str[1024];

        va_start(ap, fmt);
        vsprintf(str, fmt, ap);
        va_end(ap);

        do_report(trace_file, str);
}

#endif


void
do_securityreport(char *str, struct userec *userinfo, int fullinfo, char *addinfo)
{
        FILE *se;
        char fname[STRLEN];

        sprintf(fname, "tmp/security.%s.%05d", currentuser.userid, getpid());
        if ((se = fopen(fname, "w")) != NULL) {
                fprintf(se, "系统安全记录\n\033[1m原因：%s\033[m\n", str);
                if (addinfo)
                        fprintf(se, "%s\n", addinfo);
                if (fullinfo) {
                        fprintf(se, "\n以下是个人资料：");
                        /* Rewrite by cancel at 01/09/16 */
                        /* 修改了getuinfo()，加上了第二个参数 */
                        getuinfo(se, userinfo);
                } else {
                        getdatestring(userinfo->lastlogin);
                        fprintf(se, "\n以下是部分个人资料：\n");
                        fprintf(se, "最近光临日期 : %s\n", datestring);
                        fprintf(se, "最近光临机器 : %s\n", userinfo->lasthost);
                }
                fclose(se);
                post_security_inform(str, fname);
                unlink(fname);
        }
}

void
securityreport(char *str)
{
        do_securityreport(str, &currentuser, NA, NULL);
}

void
securityreport2(char *str, int fullinfo, char *addinfo)
{
        do_securityreport(str, &currentuser, fullinfo, addinfo);
}

/* Rewrite by cancel at 01/09/16 */
void
getuinfo(FILE *fn, struct userec *userinfo)
{
        int num;
        char buf[40];

        fprintf(fn, "\n他的代号     : %s\n", userinfo->userid);
        fprintf(fn, "他的昵称     : %s\n", userinfo->username);
        fprintf(fn, "真实姓名     : %s\n", userinfo->realname);
        fprintf(fn, "居住住址     : %s\n", userinfo->address);
        fprintf(fn, "电子邮件信箱 : %s\n", userinfo->email);
        fprintf(fn, "真实 E-mail  : %s\n", userinfo->reginfo);
        fprintf(fn, "帐号注册地址 : %s\n", userinfo->ident);
        getdatestring(userinfo->firstlogin);
        fprintf(fn, "帐号建立日期 : %s\n", datestring);
        getdatestring(userinfo->lastlogin);
        fprintf(fn, "最近光临日期 : %s\n", datestring);
        fprintf(fn, "最近光临机器 : %s\n", userinfo->lasthost);
        fprintf(fn, "上站次数     : %d 次\n", userinfo->numlogins);
        fprintf(fn, "文章数目     : %d\n", userinfo->numposts);
        fprintf(fn, "上站总时数   : %d 小时 %d 分钟\n",
                userinfo->stay / 3600, (userinfo->stay / 60) % 60);
        strcpy(buf, "bTCPRp#@XWBA#VS-DOM-F012345678");
        for (num = 0; num < 30; num++)
                if (!(userinfo->userlevel & (1 << num)))
                        buf[num] = '-';
        buf[num] = '\0';
        fprintf(fn, "使用者权限   : %s\n\n", buf);
}
/* Rewrite End. */

int post_security_inform(char *title, char *filename) {
        struct fileheader fh;
        char fname[STRLEN];
        FILE *fp1, *fp2;
        int i;
        time_t now;
	char *board = "syssecurity";

        fp1 = fopen(filename, "r");
        if (fp1 == NULL) return -1;

        i = getnewfilename(board);
        if (i == -1) return -1;
        fh.id = i;
        sprintf(fh.filename, "M.%d.A", i);
        setboardfile(fname, board, fh.filename);
        fp2 = fopen(fname, "w");
        if (fp2 == NULL) return -1;

        strlcpy(fh.owner, currentuser.userid, IDLEN + 1);
        strlcpy(fh.title, title, TITLELEN);
        fh.flag = 0;
        fh.size = file_size(filename);
        now = time(NULL);
	fh.filetime = now;
        setboardfile(fname, board, ".DIR");
        append_record(fname, &fh, sizeof(fh));

        fprintf(fp2, "发信人: %s (%s), 信区: %s\n",
                currentuser.userid, currentuser.username, board);
        fprintf(fp2, "标  题: %s\n", title);
        fprintf(fp2, "发信站: %s (%24.24s)\n", BBSNAME, ctime(&now));
	fprintf(fp2, "\n");
        file_copy(fp1, fp2);

        fclose(fp2);
        fclose(fp1);
        return 0;
}


