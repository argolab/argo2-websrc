#include "webbs.h"

static int cmd_total;
typedef struct {
        char *name;
        int (*fptr) ();
        int type;
	char *content_type;
} MENU;

static MENU cmdlist[] = {
{"login", do_login, 0, "text/html"}, /* ��½ */
{"allsec", show_all_section, 0, "text/html"}, /* ��������ͼҳ�� */
{"left", show_menu, 0, "text/html"},	      /* ��½�����������������б� */
{"boa", show_section, 0, "text/html"}, /* ?board=id����ʾ��������ͼ */
{"doc", show_board, 0, "text/html"}, /* ?board=xxx, ��ʾxx�������б� */
{"con", show_article, 0, "text/html"}, /* ?board=xx&file=filename���Ķ��������� */
{"0an", show_annpath, 0, "text/html"}, /* ?path=xxx, ��ʾ�����������б�path=/personalΪ�����ļ� */
{"anc", show_annfile, 0, "text/html"}, /* ?path=xxx����ʾ���������� */
{"gdoc", show_brddigest, 0, "text/html"}, /* ?board=xxx����ժ�б� */
{"gcon", show_digest, 0, "text/html"},	  /* ?board=xxx&start=id��xxx���idƪ��ժ */
{"tdoc", show_board_topic, 0, "text/html"}, /* ?board=xx��������ʾxx�������б� */
{"tcon", show_topic, 0, "text/html"},	    /* ?board=xx&file=filename��ͬ�����Ķ����� */
{"all", show_all_boards, 0, "text/html"}, /* �г����������� */
{"good", show_good_boards, 0, "text/html"}, /* �г��Ƽ������� */
{"qry", query_user, 0, "text/html"},	    /* ?userid=xxx����ʾxxx������Ϣ */
{"usr", show_online_user, 0, "text/html"}, /* �����û��б� */
{"sel", select_board, 0, "text/html"},	   /* ���������� */
{"gohome", show_index, 0, "text/html"},	    /* �ص���ҳ */
{"alluser", show_all_user, 0, "text/html"}, /* ��ҳ��ʾ����ʹ���� */
{"notepad", show_notepad, 0, "text/html"},  /* ���԰� */
{"reg", show_register_form, 0, "text/html"}, /* ע�����û�������ҳ */
{"not", show_board_note, 0, "text/html"},    /* ?board=xx ���滭�棨����¼�� */
{"bfind", find_inboard, 0, "text/html"},     /* ?board=xx ���ڲ�ѯ */
{"foot", show_foot, 0, "text/html"},	  /* ״̬����ʱ�䣬���ߣ��ʺţ����䣬��ͣ�� */
{"mail", show_all_mails, 0, "text/html"}, /* ��ҳ��ʾ�����ʼ��б� */
{"mailcon", show_mail, 0, "text/html"},	  /* ?start=id����ʾ��idƪ�ʼ� */
{"logout", do_logout, 0, "text/html"}, /* �ǳ� */
{"newmail", show_newmail, 0, "text/html"}, /* ��ʾ���ʼ��б� */
{"votecon", do_vote, 0, "text/html"},	   /* ?board=xx&num=id����ʾxx���id��ͶƱ���� */
{"votedoc", show_vote_list, 0, "text/html"}, /* ?board=xx, ��ʾxx��ͶƱ�б� */
{"votesnd", post_vote, 0, "text/html"},	     /* form action�ύͶƱѡ�� */
{"doreg", do_register, 0, "text/html"},	     /* form action�ύ�û����Ϲ�ע�� */
{"auth", m_activation, 0, "text/html"},	     /* �����˺� */
{"validate", m_validate, 0, "text/html"},    /* ��֤NetID��Ч�� */
{"nick", change_nick, 0, "text/html"},	   /* ��ʱ���ǳ� */
{"friend", show_friends, 0, "text/html"}, /* ���ߺ��� */
{"sig", change_signature, 0, "text/html"}, /* �޸�ǩ���� */
{"pwd", change_passwd, 0, "text/html"},	   /* �޸����� */
{"pst", write_article, 0, "text/html"},	   /* ?board=xx����xx�淢������, ?board=xx&file=filename&userid=user���ظ����� */
{"snd", submit_article, 0, "text/html"},   /* form action�ύ���·��� */
{"ball", list_bad, 0, "text/html"},    /* ���Ѻ����� */
{"fall", list_friend, 0, "text/html"}, /* �����б���趨�������� */
{"badd", add_bad, 0, "text/html"},     /* ��ӻ��� */
{"fadd", add_friend, 0, "text/html"},  /* ��Ӻ��� */
{"bdel", del_bad, 0, "text/html"},     /* ɾ������ */
{"fdel", del_friend, 0, "text/html"},  /* ɾ������ */
{"edit", edit_article, 0, "text/html"}, /* �޸����� */
{"ccc", copy_article, 0, "text/html"},	/* ת�� */
{"fwd", mail_article, 0, "text/html"},	/* ת������ */
{"del", del_article, 0, "text/html"},	/* ɾ������ */
{"denyadd", do_deny, 0, "text/html"},	/* �����µĲ���POST�û� */
{"denyall", deny_list, 0, "text/html"}, /* ĳ�汻����û��б� */
{"denydel", do_undeny, 0, "text/html"}, /* ����û� */
{"denyattach", deny_attach, 0, "text/html"}, /* ��⸽�� */
{"denymadd", m_deny, 0, "text/html"}, /* ?board=xx&num=id���Ե�idƪ���µ����߽��з�� */
{"denymdel", m_undeny, 0, "text/html"}, /* ����û� */
{"mdoc", m_show_board, 0, "text/html"}, /* ��ª�İ������ҳ */
{"pstmail", write_mail, 0, "text/html"}, /* �����ʼ�ҳ */
{"sndmail", do_send_mail, 0, "text/html"}, /* form action�ύ�ʼ����� */
{"delmail", del_mail, 0, "text/html"},	   /* ?file=filename��ɾ���ʼ� */
{"info", change_info, 0, "text/html"}, /* �޸ĸ������� */
{"clear", clear_read_flag, 0, "text/html"}, /* ����Ķ���� */
{"plan", change_plan, 0, "text/html"}, /* �޸�˵���� */
{"cloak", change_cloak, 0, "text/html"}, /* �л�����״̬ */
{"parm", change_parm, 0, "text/html"}, /* �޸ĸ��˲��� */
{"man", board_manage, 0, "text/html"}, /* ��ѡ�����¼�m/g���費�ɻظ��� */
//{"mnote", change_note, 0},
{"js", show_bar, 0},			     /* ��ӭ����[����ʱ�� Yat-sen Channel]Ŀǰ��������(www/all) [92/188] */
{"mywww", config_interface, 0, "text/html"}, /* �޸ĸ���www���� */
{"mybrd", list_favorbrd, 0, "text/html"},    /* �ղؼй��� */
{"brdadd", add_favorbrd, 0, "text/html"}, /* ��ӵ��ղؼ� */
{"stat", ranklist, 0, "text/html"},	  /* ��������ͳ�� */
{"sendmsg", send_msg, 0, "text/html"}, /* ����ѶϢ */
{"getmsg", get_msg, 0, "text/html"},   /* ��ȡѶϢ�����ʼ���ʾ */
{"delmsg", del_msg, 0, "text/html"},   /* ɾ��ѶϢ���� */
{"mailmsg", mail_msg, 0, "text/html"}, /* ��ѶϢ������ */
{"msg", show_msg, 0, "text/html"}, /* �鿴ѶϢ */
{"find", site_search, 0, "text/html"}, /* �ò��˵�ȫվ���� */
{"ufind", search_online, 0, "text/html"},  /* ufind?search=x��ʾ��x��ͷ�����û� */
{"top10", top_ten, 0, "text/html"}, /* ʮ������ */
{"topb10", board_top_ten, 0, "text/html"}, /* ʮ���������� */
{"active", show_activeboard, 0, "text/html"}, /* �壬����� */
{"sec", show_all_section, 0, "text/html"},    /* ����allsec */
{"main", show_main_frame, 0, "text/html"},    /* ��ҳ��½�������� */
{"brdcreate", create_board, 0, "text/html"},  /* �����°� */
{"attachman", upload_manage, 0, "text/html"}, /* ���渽������ */
{"attachdel", delete_attach, 0, "text/html"}, /* ɾ������ */
{"editptn", edit_pattern, 0, "text/html"},    /* �༭htmlģ�壬ò����û�� */
{"modptn", modify_pattern, 0, "text/html"},   /* ͬ�ϣ���Ϊ�ύ */
{"edituploadext", edit_uploadext, 0, "text/html"}, /* �༭ĳ�����upload.ext�ļ���ò����û�� */
{"moduploadext", modify_uploadext, 0, "text/html"}, /* ͬ�ϣ���Ϊ�ύ */
{"restart", restart_httpd, 0, "text/html"},	/* ����httpd */
{"annsearch", announce_search, 0, "text/html"}, /* ʹ��google���������� */
{"annstat_m", m_announce_stat, 0, "text/html"}, /* ����������ͳ�� */
{"boardsuggester", m_board_suggester, 0, "text/xml"}, /* ���������ֲ�ȫ */
{"useridvalidcate", m_userid_validcate, 0, "text/xml"}, /* �û����Ϸ�����֤ */
{"adsmanagement", ads_management, 0, "text/html"}, /* ������ */
{"adsupdate", ads_update, 0, "text/html"},	   /* ����ϴ� */
{"adsclick", ads_click, 0, "text/html"},	   /* ���ͳ�� */
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

	http_fatal("������ά���У���ͣʹ��");
}
