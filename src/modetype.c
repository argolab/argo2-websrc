#include "webbs.h"

char *
ModeType (mode)
int mode; {   
        switch (mode & ~WWW) {
        case IDLE:      return "";
        case NEW:       return "��վ��ע��";
        case LOGIN:     return "���뱾վ";
        case DIGEST:
	case DIGESTRACE:return "���������";
        case MMENU:     return "��ѡ��";  
        case ADMIN:     return "������ѡ��";
        case SELECT:    return "ѡ��������";
        case READBRD:   return "һ����ɽС";
        case READNEW:   return "����������";
        case READING:   return "Ʒζ����";
        case POSTING:   return "�ĺ��ӱ�";
        case MAIL:      return "�����ż�";
        case SMAIL:     return "�����Ÿ�";
        case RMAIL:     return "�����ż�";
        case TMENU:     return "����ѡ��";
        case LUSERS:    return "��������:)";
        case FRIEND:    return "Ѱ�Һ���";  
        case MONITOR:   return "̽������";
        case QUERY:     return "��ѯ����";
        case TALK:      return "����";
        case PAGE:      return "����";
        case CHAT1:     return "���ʻ�����";
        case CHAT2:     return "���Ⱥ���";
        case CHAT3:     return "Chat3"; 
        case CHAT4:     return "Chat4";   
        case LAUSERS:   return "̽������";
        case XMENU:     return "ϵͳ��Ѷ";
        case VOTING:    return "ͶƱ��...";
        case EDITUFILE: return "�༭���˵�";
        case EDITSFILE: return "����ϵͳ��";
        case ZAP:       return "����������";
        case SYSINFO:   return "���ϵͳ";
        case DICT:	return "�����ֵ�";
        case LOCKSCREEN:return "��Ļ����";
        case NOTEPAD:   return "���԰�";
        case GMENU:     return "������";
        case MSG:       return "��ѶϢ";
        case USERDEF:   return "�Զ�����";
        case EDIT:      return "�޸�����";
        case OFFLINE:   return "��ɱ��..";
        case EDITANN:   return "���޾���";
        case LOOKMSGS:  return "�쿴ѶϢ";
        case WFRIEND:   return "Ѱ������";
        case WNOTEPAD:  return "���߻���";
        case BBSNET:    return "BBSNET";
        case WINMINE:	return "����ɨ��";
        case FIVE:	return "��ս������";
        case WORKER:	return "������";
        case PAGE_FIVE:	return "��������";
	case MOBILE:    return "�ֻ�ģʽ";
        default:        return "ȥ���Ķ�??";
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
