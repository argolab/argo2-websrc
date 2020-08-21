#include "webbs.h"

static int cmd_total;
typedef struct {
        char *name;
        int (*fptr) ();
        int type;
	char *content_type;
} MENU;

static MENU cmdlist[] = {
{"login", do_login, 0, "text/html"}, /* 登陆 */
{"allsec", show_all_section, 0, "text/html"}, /* 讨论区地图页面 */
{"left", show_menu, 0, "text/html"},	      /* 登陆后左侧边栏，即命令列表 */
{"boa", show_section, 0, "text/html"}, /* ?board=id，显示讨论区地图 */
{"doc", show_board, 0, "text/html"}, /* ?board=xxx, 显示xx版文章列表 */
{"con", show_article, 0, "text/html"}, /* ?board=xx&file=filename，阅读单个帖子 */
{"0an", show_annpath, 0, "text/html"}, /* ?path=xxx, 显示精华区文章列表，path=/personal为个人文集 */
{"anc", show_annfile, 0, "text/html"}, /* ?path=xxx，显示精华区文章 */
{"gdoc", show_brddigest, 0, "text/html"}, /* ?board=xxx，文摘列表 */
{"gcon", show_digest, 0, "text/html"},	  /* ?board=xxx&start=id，xxx版第id篇文摘 */
{"tdoc", show_board_topic, 0, "text/html"}, /* ?board=xx，主题显示xx版文章列表 */
{"tcon", show_topic, 0, "text/html"},	    /* ?board=xx&file=filename，同主题阅读帖子 */
{"all", show_all_boards, 0, "text/html"}, /* 列出所有讨论区 */
{"good", show_good_boards, 0, "text/html"}, /* 列出推荐讨论区 */
{"qry", query_user, 0, "text/html"},	    /* ?userid=xxx，显示xxx个人信息 */
{"usr", show_online_user, 0, "text/html"}, /* 在线用户列表 */
{"sel", select_board, 0, "text/html"},	   /* 查找讨论区 */
{"gohome", show_index, 0, "text/html"},	    /* 回到主页 */
{"alluser", show_all_user, 0, "text/html"}, /* 分页显示所有使用者 */
{"notepad", show_notepad, 0, "text/html"},  /* 留言版 */
{"reg", show_register_form, 0, "text/html"}, /* 注册新用户填资料页 */
{"not", show_board_note, 0, "text/html"},    /* ?board=xx 进版画面（备忘录） */
{"bfind", find_inboard, 0, "text/html"},     /* ?board=xx 版内查询 */
{"foot", show_foot, 0, "text/html"},	  /* 状态栏：时间，在线，帐号，信箱，已停留 */
{"mail", show_all_mails, 0, "text/html"}, /* 分页显示所有邮件列表 */
{"mailcon", show_mail, 0, "text/html"},	  /* ?start=id，显示第id篇邮件 */
{"logout", do_logout, 0, "text/html"}, /* 登出 */
{"newmail", show_newmail, 0, "text/html"}, /* 显示新邮件列表 */
{"votecon", do_vote, 0, "text/html"},	   /* ?board=xx&num=id，显示xx版第id个投票内容 */
{"votedoc", show_vote_list, 0, "text/html"}, /* ?board=xx, 显示xx版投票列表 */
{"votesnd", post_vote, 0, "text/html"},	     /* form action提交投票选项 */
{"doreg", do_register, 0, "text/html"},	     /* form action提交用户资料供注册 */
{"auth", m_activation, 0, "text/html"},	     /* 激活账号 */
{"validate", m_validate, 0, "text/html"},    /* 验证NetID有效性 */
{"nick", change_nick, 0, "text/html"},	   /* 临时改昵称 */
{"friend", show_friends, 0, "text/html"}, /* 在线好友 */
{"sig", change_signature, 0, "text/html"}, /* 修改签名档 */
{"pwd", change_passwd, 0, "text/html"},	   /* 修改密码 */
{"pst", write_article, 0, "text/html"},	   /* ?board=xx，在xx版发表文章, ?board=xx&file=filename&userid=user，回复文章 */
{"snd", submit_article, 0, "text/html"},   /* form action提交文章发表 */
{"ball", list_bad, 0, "text/html"},    /* 好友黑名单 */
{"fall", list_friend, 0, "text/html"}, /* 好友列表兼设定好友名单 */
{"badd", add_bad, 0, "text/html"},     /* 添加坏人 */
{"fadd", add_friend, 0, "text/html"},  /* 添加好友 */
{"bdel", del_bad, 0, "text/html"},     /* 删除坏人 */
{"fdel", del_friend, 0, "text/html"},  /* 删除好友 */
{"edit", edit_article, 0, "text/html"}, /* 修改文章 */
{"ccc", copy_article, 0, "text/html"},	/* 转帖 */
{"fwd", mail_article, 0, "text/html"},	/* 转寄文章 */
{"del", del_article, 0, "text/html"},	/* 删除文章 */
{"denyadd", do_deny, 0, "text/html"},	/* 设置新的不可POST用户 */
{"denyall", deny_list, 0, "text/html"}, /* 某版被封禁用户列表 */
{"denydel", do_undeny, 0, "text/html"}, /* 解封用户 */
{"denyattach", deny_attach, 0, "text/html"}, /* 解封附文 */
{"denymadd", m_deny, 0, "text/html"}, /* ?board=xx&num=id，对第id篇文章的作者进行封禁 */
{"denymdel", m_undeny, 0, "text/html"}, /* 解封用户 */
{"mdoc", m_show_board, 0, "text/html"}, /* 丑陋的版面管理页 */
{"pstmail", write_mail, 0, "text/html"}, /* 发送邮件页 */
{"sndmail", do_send_mail, 0, "text/html"}, /* form action提交邮件发送 */
{"delmail", del_mail, 0, "text/html"},	   /* ?file=filename，删除邮件 */
{"info", change_info, 0, "text/html"}, /* 修改个人资料 */
{"clear", clear_read_flag, 0, "text/html"}, /* 清除阅读标记 */
{"plan", change_plan, 0, "text/html"}, /* 修改说明档 */
{"cloak", change_cloak, 0, "text/html"}, /* 切换隐身状态 */
{"parm", change_parm, 0, "text/html"}, /* 修改个人参数 */
{"man", board_manage, 0, "text/html"}, /* 对选定文章加m/g或设不可回复等 */
//{"mnote", change_note, 0},
{"js", show_bar, 0},			     /* 欢迎访问[逸仙时空 Yat-sen Channel]目前在线人数(www/all) [92/188] */
{"mywww", config_interface, 0, "text/html"}, /* 修改个人www参数 */
{"mybrd", list_favorbrd, 0, "text/html"},    /* 收藏夹管理 */
{"brdadd", add_favorbrd, 0, "text/html"}, /* 添加到收藏夹 */
{"stat", ranklist, 0, "text/html"},	  /* 个人排名统计 */
{"sendmsg", send_msg, 0, "text/html"}, /* 发送讯息 */
{"getmsg", get_msg, 0, "text/html"},   /* 收取讯息或新邮件提示 */
{"delmsg", del_msg, 0, "text/html"},   /* 删除讯息备份 */
{"mailmsg", mail_msg, 0, "text/html"}, /* 寄讯息回邮箱 */
{"msg", show_msg, 0, "text/html"}, /* 查看讯息 */
{"find", site_search, 0, "text/html"}, /* 用不了的全站查找 */
{"ufind", search_online, 0, "text/html"},  /* ufind?search=x显示以x开头在线用户 */
{"top10", top_ten, 0, "text/html"}, /* 十大帖子 */
{"topb10", board_top_ten, 0, "text/html"}, /* 十大人气版面 */
{"active", show_activeboard, 0, "text/html"}, /* 囧，活动看版 */
{"sec", show_all_section, 0, "text/html"},    /* 等于allsec */
{"main", show_main_frame, 0, "text/html"},    /* 主页登陆后的主框架 */
{"brdcreate", create_board, 0, "text/html"},  /* 创建新版 */
{"attachman", upload_manage, 0, "text/html"}, /* 版面附件管理 */
{"attachdel", delete_attach, 0, "text/html"}, /* 删除附件 */
{"editptn", edit_pattern, 0, "text/html"},    /* 编辑html模板，貌似已没用 */
{"modptn", modify_pattern, 0, "text/html"},   /* 同上，此为提交 */
{"edituploadext", edit_uploadext, 0, "text/html"}, /* 编辑某个版的upload.ext文件，貌似已没用 */
{"moduploadext", modify_uploadext, 0, "text/html"}, /* 同上，此为提交 */
{"restart", restart_httpd, 0, "text/html"},	/* 重启httpd */
{"annsearch", announce_search, 0, "text/html"}, /* 使用google搜索精华区 */
{"annstat_m", m_announce_stat, 0, "text/html"}, /* 精华区操作统计 */
{"boardsuggester", m_board_suggester, 0, "text/xml"}, /* 讨论区名字补全 */
{"useridvalidcate", m_userid_validcate, 0, "text/xml"}, /* 用户名合法性验证 */
{"adsmanagement", ads_management, 0, "text/html"}, /* 广告管理 */
{"adsupdate", ads_update, 0, "text/html"},	   /* 广告上传 */
{"adsclick", ads_click, 0, "text/html"},	   /* 广告统计 */
{0, 0, 0}
};

/* babydragon: sort the cmdlist when httpd start, then we can use bin-search */
void cmd_sort() {
	int i, j;
	MENU tmp;
	int min;
	for (i = 0; cmdlist[i].name != 0; i++) {
		min = i;
		for (j = i + 1; cmdlist[j].name != 0; j++) {
			if (strcmp(cmdlist[min].name, cmdlist[j].name) > 0) {
				min = j;
			}
		}
		tmp = cmdlist[i];
		cmdlist[i] = cmdlist[min];
		cmdlist[min] = tmp;
	}
	cmd_total = i;
}

int cmd_search(char *name) {
	int left = 0;
	int right = cmd_total - 1;
	int middle, rt;
	while (left <= right) {
		middle = (left + right)/2;
		rt = strcmp(name, cmdlist[middle].name);
		if (rt < 0) {
			right = middle - 1;
		} else if (rt == 0) {
			return middle;
		} else {
			left = middle + 1;
		}
	}
	return -1;
}

int do_stuff() {
	int i;
	char *nameptr = &stuff_ent[3];
	/*
	for (i = 0; cmdlist[i].name; i++) {
		if (!strcmp(cmdlist[i].name, nameptr)) {
			http_msg(200);
			printf("Content-type: text/html\r\n\r\n");
			fflush(stdout);
			(cmdlist[i].fptr)();
			fflush(stdout);
			return 0;
		}
	}
	http_msg(404);
	*/
	if ( (i = cmd_search(nameptr)) >= 0) {
		http_msg(200);
		printf("Content-type: %s\r\n\r\n", cmdlist[i].content_type);
		fflush(stdout);
		(cmdlist[i].fptr)();
		fflush(stdout);
		return 0;
	} else {
		http_msg(404);
	}
	return -1;
}

void maintaining() {
	init_all();

	http_fatal("本功能维护中，暂停使用");
}
