#include "webbs.h"

int 
has_enter_perm (user, board)
struct userec *user;
char *board; {
        struct boardheader *x;
        if(!has_read_perm(user, board)) return 0;
        x=getbcache(board);
        if(x==0) return 0; 
        if(!loginok) return 0;
        if(!strcasecmp(board, "sysop")) return 1;
        if(user_perm(user, PERM_SYSOP)) return 1;
        if(!user_perm(user, PERM_BASIC)) return 0;  
        return 1;
}

int 
has_post_perm (user, board)
struct userec *user;
char *board;
{
        struct boardheader *bp;
        struct denyheader dh;
        char filename[STRLEN];
        int num;
        
        if (!loginok || !has_read_perm(user, board))
                return NA;
                
        if ((bp = getbcache(board)) == NULL)
                return NA;
	
	/* 未激活id不可发帖 */ 
	if (!( bp -> level & PERM_POSTMASK)) 
		if ( !user_perm(user, PERM_WELCOME) )
			return NA;

        if (!strcasecmp(board, DEFAULTBOARD)) 	/*  complain */
		return YEA; //henry 02.06.17
        
        // monster: 检查是否为只读版
        if (bp->flag & BRD_READONLY)
                return NA;

       	if ( user_perm(user, PERM_SYSOP) )
		return YEA;

	/* freestyler: 推荐版不能发文 */
	if (!strcmp(board, DEFAULTRECOMMENDBOARD) && !has_BM_perm(&currentuser, board) ) {
		return NA;
	}

        // monster: 检查是否达到文章上限
        if (!has_BM_perm(&currentuser, board) && !(bp->flag & NOPLIMIT_FLAG)) {
                sprintf(filename, "boards/%s/.DIR", board);
                num = get_num_records(filename, sizeof (struct fileheader));
                if ((!(bp->flag & BRD_MAXII_FLAG) && num >= MAX_BOARD_POST) ||
                    ((bp->flag & BRD_MAXII_FLAG) && num >= MAX_BOARD_POST_II))
                        return NA;
        }

	/* added by freestyler 08.06.23 
	 先检查版面属性，再检查POST权，让一些未获得POST权的用户
	 依然可以在某些特定版发文，这类用户包括未通过注册的和被
	 封全站的。目前代码对这两类用户不作区分 */
	if ( bp -> level & PERM_POSTMASK)
		return HAS_PERM(( bp->level & ~PERM_NOZAP) & ~PERM_POSTMASK);

        if(!user_perm(user, PERM_POST)) return 0;

        sprintf(filename, "boards/%s/.DENYLIST", board);
        return (search_record(filename, (void *)&dh, sizeof(struct denyheader), deny_names, (void *)user->userid)) ?
                NA : (user_perm(user, bp->level) ? YEA : bp->level ? NA : YEA);
}

/* Henry: check PERM_SENDMAIL 02.07.11 */
int 
has_mail_perm (user)
struct userec *user; {
        if(!loginok) return 0;
        if(user_perm(user, PERM_SYSOP)) return 1;
        if(!user_perm(user, PERM_BASIC)) return 0;
        /* babydragon: not to check PERM_SENDMAIL 2006.10.8*/
	//if(!user_perm(user, PERM_SENDMAIL)) return 0;
	// henry: 校外ip不能发信
//	if (!validate_ip_range(fromhost) && !user_perm(user, PERM_OBOARDS)) return NA;
        return 1;
}

int 
has_BM_perm (user, board)
struct userec *user;
char *board; {
        struct boardheader *x;
        char buf[256], *bm;

	// henry：校外ip不能管理版面
	// 现在杨老师说可以了
	// if (!validate_ip_range(fromhost)) return NA;
        x=getbcache(board);
        if(x==0) return 0;
        if(user_perm(user, PERM_BLEVELS)) return 1;
        if(!user_perm(user, PERM_BOARDS)) return 0;
        strcpy(buf, x->BM);
        bm=strtok(buf, ",: ;&()\n");
        while(bm) {
                if(!strcasecmp(bm, user->userid)) return 1;
                bm=strtok(0, ",: ;&()\n");
        }
        return 0;
}

int 
has_read_perm (user, board)
struct userec *user;
char *board; {
	struct boardheader *x;    /* 版面不存在返回0, p和z版面返回1, 有权限版面返回1. */
	if(board[0]<=32) return 0;
        x=getbcache(board);
        if(x==0) return 0; 
        
        if (x->flag & BRD_RESTRICT) {
                char boardctl[STRLEN];   
                
                sethomefile(boardctl, currentuser.userid, "board.ctl");
                if (seek_in_file(boardctl, board) == 0)
                        return 0;
        }

	if ( !validate_ip_range(fromhost) ) {
		/* 校内版面 */
		if( x->flag & BRD_INTERN )   {
			if(!user_perm(user, PERM_SYSOP) 
			&& !user_perm(user, PERM_BOARDS)
			&& !user_perm(user, PERM_INTERNAL) ) 
		        	return 0;
		}
		/* freestyler: 半开放版面 */
		if ( x->flag & BRD_HALFOPEN ) {
			if( !user_perm(user, PERM_SYSOP)
		 	 && !user_perm(user, PERM_WELCOME)) 
				return 0;
		}
	}

	if(x->level==0) return 1;
        if(x->level & (PERM_POSTMASK | PERM_NOZAP)) return 1;
        if(!user_perm(user, PERM_BASIC)) return 0;
        if(user_perm(user, x->level)) return 1;
        return 0;
}

int 
has_post_perm1 (user, board)
struct userec *user;
char *board; {
        char buf3[256];
        struct boardheader *x;
        if(!has_read_perm(user, board)) return 0;
        x=getbcache(board);
        if(x==0) return 0;
        if(!loginok) return 0;
        sprintf(buf3, "boards/%s/deny_users", x->filename);
        if(seek_in_file(buf3, user->userid)) return 0;
        //if(!strcasecmp(board, "sysop")) return 1;
        if(user_perm(user, PERM_SYSOP)) return 1;
        if(!user_perm(user, PERM_BASIC)) return 0;
        if(!user_perm(user, PERM_POST)) return 0;
        if(!(x->level & PERM_NOZAP) && x->level && !user_perm(user, x->level)) return 0;
        return 1;
}

int 
user_perm (x, level)
struct userec *x;
int level; {
        return (x->userlevel & level);
}

//Added by cancel At 02.05.22, rewrite by Henry 03.02.15
int 
deny_names (userid, dh)
void *userid;
void *dh;
{
        char buf[IDLEN+2], *ptr;

        strlcpy(buf, ((struct denyheader *)dh)->blacklist, sizeof(buf));
	ptr = strtok(buf, " ");
	if (ptr == NULL) return 0;
        return (strcmp((char *)userid, ptr)) ? 0 : 1; 
}

int denynames(void *userid, void *dh)
{
        return (strncmp((char *)userid, ((struct denyheader *)dh)->blacklist, IDLEN)) ? 0 : 1;
}
 
int deny_me(char *bname)
{
        char buf[STRLEN];
        struct denyheader dh;
        
        sprintf(buf, "boards/%s/.DENYLIST", bname);
        return (search_record(buf, &dh, sizeof(struct denyheader), denynames,
                currentuser.userid));
}

int
is_rejected (id)
char *id;
{
        struct override *rec;
        char buf[128];
        int nr=0, i;
	int result;

        if (HAS_PERM(PERM_SYSOP)) return NA;
        sethomefile(buf, id, "rejects");
        nr = file_size(buf)/sizeof (struct override);
        if (nr <= 0) return 0;
        rec = (struct override *) calloc(sizeof (struct override), nr);
        if (get_records(buf, rec, sizeof(struct override), 1, nr) == -1) return 0;
	result = 0;
        for (i=0; i<nr; i++)
                if (!strncmp(currentuser.userid, rec[i].id,
                    strlen(currentuser.userid)))
                        {
				result = 1;
				break;
			}
	free(rec);
	return result;
}
