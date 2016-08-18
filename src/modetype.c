#include "webbs.h"

char *
ModeType (mode)
int mode; {   
        switch (mode & ~WWW) {
        case IDLE:      return "";
        case NEW:       return "新站友注册";
        case LOGIN:     return "进入本站";
        case DIGEST:
	case DIGESTRACE:return "浏览精华区";
        case MMENU:     return "主选单";  
        case ADMIN:     return "管理者选单";
        case SELECT:    return "选择讨论区";
        case READBRD:   return "一览众山小";
        case READNEW:   return "看看新文章";
        case READING:   return "品味文章";
        case POSTING:   return "文豪挥笔";
        case MAIL:      return "处理信笺";
        case SMAIL:     return "寄语信鸽";
        case RMAIL:     return "阅览信笺";
        case TMENU:     return "聊天选单";
        case LUSERS:    return "东张西望:)";
        case FRIEND:    return "寻找好友";  
        case MONITOR:   return "探视民情";
        case QUERY:     return "查询网友";
        case TALK:      return "聊天";
        case PAGE:      return "呼叫";
        case CHAT1:     return "国际会议厅";
        case CHAT2:     return "咖啡红茶馆";
        case CHAT3:     return "Chat3"; 
        case CHAT4:     return "Chat4";   
        case LAUSERS:   return "探视网友";
        case XMENU:     return "系统资讯";
        case VOTING:    return "投票中...";
        case EDITUFILE: return "编辑个人档";
        case EDITSFILE: return "编修系统档";
        case ZAP:       return "订阅讨论区";
        case SYSINFO:   return "检查系统";
        case DICT:	return "翻查字典";
        case LOCKSCREEN:return "屏幕锁定";
        case NOTEPAD:   return "留言板";
        case GMENU:     return "工具箱";
        case MSG:       return "送讯息";
        case USERDEF:   return "自订参数";
        case EDIT:      return "修改文章";
        case OFFLINE:   return "自杀中..";
        case EDITANN:   return "编修精华";
        case LOOKMSGS:  return "察看讯息";
        case WFRIEND:   return "寻人名册";
        case WNOTEPAD:  return "欲走还留";
        case BBSNET:    return "BBSNET";
        case WINMINE:	return "键盘扫雷";
        case FIVE:	return "决战五子棋";
        case WORKER:	return "推箱子";
        case PAGE_FIVE:	return "邀请下棋";
	case MOBILE:    return "手机模式";
        default:        return "去了哪儿??";
        }
}

void modify_user_mode(int mode) {
	if (u_info == NULL) return;
	u_info->mode = WWW | mode;
}

int www_mode(struct user_info *u) {
	if (u == NULL) return 0;
	return (u->mode & WWW);
}
