#include "webbs.h"

char brc_buf[BRC_MAXSIZE];
int brc_cur, brc_size, brc_changed = 0;
char brc_name[BRC_STRLEN];
int brc_list[BRC_MAXNUM], brc_num;
char currboard[STRLEN];

/* monster: adopted from ytht */
char *
brc_getrecord (ptr, name, pnum, list)
char *ptr;
char *name;
int *pnum;
int *list;
{
	int num;
	char *tmp;

	strlcpy(name, ptr, BRC_STRLEN);
	ptr += BRC_STRLEN;
	num = (*ptr++) & 0xff;
	tmp = ptr + num * sizeof (int);
	if (num > BRC_MAXNUM) {
		num = BRC_MAXNUM;
	}
	*pnum = num;
	memcpy(list, ptr, num * sizeof (int));
	return tmp;
}

char *
brc_putrecord (ptr, name, num, list)
char *ptr;
char *name;
int num;
int *list;
{
	if (num > 0) {
		if (num > BRC_MAXNUM) {
			num = BRC_MAXNUM;
		}
		strlcpy(ptr, name, BRC_STRLEN);
		ptr += BRC_STRLEN;
		*ptr++ = num;
		memcpy(ptr, list, num * sizeof (int));
		ptr += num * sizeof (int);
	}
	return ptr;
}

void 
brc_update ()
{
	char dirfile[STRLEN], *ptr;
	char tmp_buf[BRC_MAXSIZE - BRC_ITEMSIZE], *tmp;
	char tmp_name[BRC_STRLEN];
	int tmp_list[BRC_MAXNUM], tmp_num;
	int fd, tmp_size;

	if (brc_changed == 0 || !strcmp(currentuser.userid, "guest"))
		return;
	ptr = brc_buf;
	if (brc_num > 0) {
		ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);
	}
	if (1) {
		sprintf(dirfile, "home/%c/%s/.boardrc", mytoupper(currentuser.userid[0]), currentuser.userid);
		if ((fd = open(dirfile, O_RDONLY)) != -1) {
			tmp_size = read(fd, tmp_buf, sizeof (tmp_buf));
			close(fd);
		} else {
			tmp_size = 0;
		}
	}
	tmp = tmp_buf;
	while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z')) {
		tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
		if (strncmp(tmp_name, currboard, BRC_STRLEN) != 0) {
			ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
		}
	}
	brc_size = (int) (ptr - brc_buf);
	
	if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
		ftruncate(fd, 0);
		write(fd, brc_buf, brc_size);
		close(fd);
	}
	brc_changed = 0;
}

int 
brc_initial (boardname)
char *boardname;
{
	char dirfile[STRLEN], *ptr;
	int fd;

	brc_cur=0;brc_size=0;brc_changed = 0;
	brc_name[0]='\0';
	brc_buf[0]='\0';
	brc_num=0;

	strlcpy(currboard, boardname, sizeof(currboard));
	brc_changed = 0;
	if (brc_buf[0] == '\0') {
		setuserfile(dirfile, ".boardrc");
		if ((fd = open(dirfile, O_RDONLY)) != -1) {
			brc_size = read(fd, brc_buf, sizeof (brc_buf));
			close(fd);
		} else {
			brc_size = 0;
		}
	}
	brc_cur = 0;
	ptr = brc_buf;
	while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z')) {
		ptr = brc_getrecord(ptr, brc_name, &brc_num, brc_list);
		if (strncmp(brc_name, currboard, BRC_STRLEN) == 0) {
			return brc_num;
		}
	}
	strlcpy(brc_name, boardname, BRC_STRLEN);
	brc_list[0] = 1;
	brc_num = 1;
	return 0;
}

/* 如果找到, num==brc_list[brc_cur]
 * 否则, brc_cur指向num应该插入的位置 
 *                              ecnegrevid 2001.6.18
 */
int 
brc_locate (num)
int num;
{
	if (brc_num == 0) {
		brc_cur = 0;
		return 0;
	}
	if (brc_cur >= brc_num)
		brc_cur = brc_num - 1;
	if (num <= brc_list[brc_cur]) {
		while (brc_cur < brc_num) {
			if (num == brc_list[brc_cur])
				return 1;
			if (num > brc_list[brc_cur])
				return 0;
			brc_cur++;
		}
		return 0;
	}
	while (brc_cur > 0) {
		if (num < brc_list[brc_cur - 1])
			return 0;
		brc_cur--;
		if (num == brc_list[brc_cur])
			return 1;
	}
	return 0;
}

/* 将num插入到brc_cur指示的位置 */
int 
brc_insert (num)
int num;
{
	if (brc_num < BRC_MAXNUM)
		brc_num++;
	if (brc_cur >= brc_num)
		return -1;
	memmove(&brc_list[brc_cur + 1], &brc_list[brc_cur],
		sizeof (brc_list[0]) * (brc_num - brc_cur - 1));
	brc_list[brc_cur] = num;
	brc_changed = 1;
	return 0;
}

void 
brc_addlist (filename)
char *filename;
{
	int ftime;

	ftime = atoi(&filename[2]);
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.') {
		return;
	}
	if (brc_unreadt(ftime))
		brc_insert(ftime);
}

int 
brc_unread (filename)
char *filename;
{
	int ftime;

	ftime = atoi(&filename[2]);
	if ((filename[0] != 'M' && filename[0] != 'G') || filename[1] != '.') {
		return 0;
	}
	return brc_unreadt(ftime);
}

int 
brc_unreadt (ftime)
int ftime;
{
	if (brc_locate(ftime)) {
		return 0;
	}
	if (brc_num <= 0) {
		return 1;
	}
	if (brc_cur < brc_num) {
		return 1;
	}
	return 0;
}

void 
brc_clear ()
{
        brc_num = 0;
        brc_cur = 0;
        brc_insert(time(0));
}

int 
get_lastpost (board, lastpost, total)
char *board;
int *lastpost;
int *total;
{
        struct fileheader fh;
        struct stat st;
        char filename[STRLEN];
        int fd, atotal;
        
	setboardfile(filename, board, ".DIR");

        if (stat(filename, &st) == -1 || st.st_size == 0 ||
            (fd = open(filename, O_RDONLY)) < 0)
                return 0;
        
        atotal = st.st_size / sizeof (fh);
        *total = atotal;
        lseek(fd, (off_t) (atotal - 1) * sizeof (fh), SEEK_SET);
        if (read(fd, &fh, sizeof (fh)) > 0) {
                *lastpost = fh.filetime;
        }
        close(fd);
        return 0;
}

int 
update_lastpost (board)
char *board;
{  
        struct boardheader *bptr;
        
        bptr = getbcache(board);
        if (bptr == NULL)
                return -1;
        get_lastpost(bptr->filename, &bptr->lastpost, &bptr->total);
        return 0; 
}

struct boardheader *
getbcache (board)
char *board; {
        int i;
        if(board == NULL) return NULL;
        for(i=0; i<MAXBOARD; i++)
                if (!strcasecmp(board, shm_bcache->bcache[i].filename))
                        return &shm_bcache->bcache[i];
        return NULL;
}
