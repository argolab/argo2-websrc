/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU

    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
			Guy Vega, gtvega@seabass.st.usm.edu
			Dominic Tynes, dbtynes@seabass.st.usm.edu

    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
			Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

    Firebird2000 Bulletin Board System
    Copyright (C) 1999-2001, deardragon, dragon@fb2000.dhs.org

    Puke Bulletin Board System
    Copyright (C) 2001-2002, Yu Chen, monster@marco.sysu.edu.cn
			     Bin Jie Lee, is99lbj@student.sysu.edu.cn

    Contains codes from YTHT & SMTH distributions of Firebird Bulletin
    Board System.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "webbs.h"

#define SC_BUFSIZE              20480
#define SC_KEYSIZE              256
#define SC_CMDSIZE              256
#define sysconf_ptr( offset )   (&sysconf_buf[ offset ]);

static struct smenuitem {
	int line, col, level;
	char *name, *desc, *arg;
} *menuitem;

static struct sdefine {
	char *key, *str;
	int val;
} *sysvar;

static char *sysconf_buf;
static int sysconf_menu, sysconf_key, sysconf_len;

void
encodestr(char *str)
{
	char ch, *buf;
	int n;

	buf = str;
	while ((ch = *str++) != '\0') {
		if (*str == ch && str[1] == ch && str[2] == ch) {
			n = 4;
			str += 3;
			while (*str == ch && n < 100) {
				str++;
				n++;
			}
			*buf++ = '\01';
			*buf++ = ch;
			*buf++ = n;
		} else
			*buf++ = ch;
	}
	*buf = '\0';
}

void *
sysconf_addstr(char *str)
{
	int len = sysconf_len;
	char *buf;

	buf = sysconf_buf + len;
	strcpy(buf, str);
	sysconf_len = len + strlen(str) + 1;
	return buf;
}

char *
sysconf_str(char *key)
{
	int n;

	for (n = 0; n < sysconf_key; n++)
		if (strcmp(key, sysvar[n].key) == 0)
			return (sysvar[n].str);
	return NULL;
}

int
sysconf_eval(char *key)
{
	int n;

	for (n = 0; n < sysconf_key; n++)
		if (strcmp(key, sysvar[n].key) == 0)
			return (sysvar[n].val);
	if (*key < '0' || *key > '9') {
		report("sysconf: unknown key: %s.", key);
	}
	return (strtol(key, NULL, 0));
}

void
sysconf_addkey(char *key, char *str, int val)
{
	int num;

	if (sysconf_key < SC_KEYSIZE) {
		if (str == NULL)
			str = sysconf_buf;
		else
			str = sysconf_addstr(str);
		num = sysconf_key++;
		sysvar[num].key = sysconf_addstr(key);
		sysvar[num].str = str;
		sysvar[num].val = val;
	}
}

void
sysconf_addmenu(FILE *fp, char *key)
{
	struct smenuitem *pm;
	char buf[256];
	char *cmd, *arg[5], *ptr;
	int n;

	sysconf_addkey(key, "menu", sysconf_menu);
	while (fgets(buf, sizeof (buf), fp) != NULL && buf[0] != '%') {
		cmd = strtok(buf, " \t\n");
		if (cmd == NULL || *cmd == '#') {
			continue;
		}
		arg[0] = arg[1] = arg[2] = arg[3] = arg[4] = "";
		n = 0;
		for (n = 0; n < 5; n++) {
			if ((ptr = strtok(NULL, ",\n")) == NULL)
				break;
			while (*ptr == ' ' || *ptr == '\t')
				ptr++;
			if (*ptr == '"') {
				arg[n] = ++ptr;
				while (*ptr != '"' && *ptr != '\0')
					ptr++;
				*ptr = '\0';
			} else {
				arg[n] = ptr;
				while (*ptr != ' ' && *ptr != '\t' &&
				       *ptr != '\0')
					ptr++;
				*ptr = '\0';
			}
		}
		pm = &menuitem[sysconf_menu++];
		pm->line = sysconf_eval(arg[0]);
		pm->col = sysconf_eval(arg[1]);
		if (*cmd == '@') {
			pm->level = sysconf_eval(arg[2]);
			pm->name = sysconf_addstr(arg[3]);
			pm->desc = sysconf_addstr(arg[4]);
			pm->arg = pm->name;
		} else if (*cmd == '!') {
			pm->level = sysconf_eval(arg[2]);
			pm->name = sysconf_addstr(arg[3]);
			pm->desc = sysconf_addstr(arg[4]);
			pm->arg = sysconf_addstr(cmd + 1);
		} else {
			pm->level = -2;
			pm->name = sysconf_addstr(cmd);
			pm->desc = sysconf_addstr(arg[2]);
			pm->arg = sysconf_buf;
		}
	}
	pm = &menuitem[sysconf_menu++];
	pm->name = pm->desc = pm->arg = sysconf_buf;
	pm->level = -1;
}

void
sysconf_addblock(FILE *fp, char *key)
{
	char buf[256];
	int num;

	if (sysconf_key < SC_KEYSIZE) {
		num = sysconf_key++;
		sysvar[num].key = sysconf_addstr(key);
		sysvar[num].str = sysconf_buf + sysconf_len;
		sysvar[num].val = -1;
		while (fgets(buf, sizeof (buf), fp) != NULL && buf[0] != '%') {
			encodestr(buf);
			strcpy(sysconf_buf + sysconf_len, buf);
			sysconf_len += strlen(buf);
		}
		sysconf_len++;
	} else {
		while (fgets(buf, sizeof (buf), fp) != NULL && buf[0] != '%')
			;
	}
}

void
parse_sysconf(char *fname)
{
	FILE *fp;
	char buf[256];
	char tmp[256], *ptr;
	char *key, *str;
	int val;

	if ((fp = fopen(fname, "r")) == NULL)
		return;

	sysconf_addstr("(null ptr)");
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		if (*ptr == '%') {
			strtok(ptr, " \t\n");
			if (strcmp(ptr, "%menu") == 0) {
				if ((str = strtok(NULL, " \t\n")) != NULL)
					sysconf_addmenu(fp, str);
			} else {
				sysconf_addblock(fp, ptr + 1);
			}
		} else if (*ptr == '#') {
			if ((key = strtok(ptr, " \t\"\n")) != NULL && 
			    (str = strtok(NULL, " \t\"\n")) != NULL && 
			     strcmp(key, "#include") == 0)
				parse_sysconf(str);
		} else if (*ptr != '\n') {
			if ((key = strtok(ptr, "=#\n")) != NULL && (str = strtok(NULL, "#\n")) != NULL) {
				strtok(key, " \t");
				while (*str == ' ' || *str == '\t')
					str++;
				if (*str == '"') {
					str++;
					strtok(str, "\"");
					val = atoi(str);
					sysconf_addkey(key, str, val);
				} else {
					val = 0;
					strcpy(tmp, str);
					ptr = strtok(tmp, ", \t");
					while (ptr != NULL) {
						val |= sysconf_eval(ptr);
						ptr = strtok(NULL, ", \t");
					}
					sysconf_addkey(key, NULL, val);
				}
			} else {
				report(ptr);
			}
		}
	}
	fclose(fp);
}

void
build_sysconf(char *configfile, char *imgfile)
{
	struct smenuitem *old_menuitem;
	struct sdefine *old_sysvar;
	char *old_buf;
	int old_menu, old_key, old_len;
	struct sysheader {
		char *buf;
		int menu, key, len;
	} shead;
	int fh;

	old_menuitem = menuitem;
	old_menu = sysconf_menu;
	old_sysvar = sysvar;
	old_key = sysconf_key;
	old_buf = sysconf_buf;
	old_len = sysconf_len;
	menuitem = (void *) malloc(SC_CMDSIZE * sizeof (struct smenuitem));
	sysvar = (void *) malloc(SC_KEYSIZE * sizeof (struct sdefine));
	sysconf_buf = (void *) malloc(SC_BUFSIZE);
	sysconf_menu = 0;
	sysconf_key = 0;
	sysconf_len = 0;
	parse_sysconf(configfile);
	if ((fh = open(imgfile, O_WRONLY | O_CREAT, 0644)) > 0) {
		ftruncate(fh, 0);
		shead.buf = sysconf_buf;
		shead.menu = sysconf_menu;
		shead.key = sysconf_key;
		shead.len = sysconf_len;
		write(fh, &shead, sizeof (shead));
		write(fh, menuitem, sysconf_menu * sizeof (struct smenuitem));
		write(fh, sysvar, sysconf_key * sizeof (struct sdefine));
		write(fh, sysconf_buf, sysconf_len);
		close(fh);
	}
	free(menuitem);
	free(sysvar);
	free(sysconf_buf);
	menuitem = old_menuitem;
	sysconf_menu = old_menu;
	sysvar = old_sysvar;
	sysconf_key = old_key;
	sysconf_buf = old_buf;
	sysconf_len = old_len;
}

void
load_sysconf_image(char *imgfile, int rebuild) 
{
	struct sysheader {
		char *buf;
		int menu, key, len;
	} shead;
	struct stat st;
	char *ptr;
	int fh, n, diff;

rebuild:
	if (rebuild == YEA) {
		report("build & reload sysconf.img");
		build_sysconf("etc/sysconf.ini", "sysconf.img");
	}

	if ((fh = open(imgfile, O_RDONLY)) > 0) {
		fstat(fh, &st);
		ptr = malloc(st.st_size);
		read(fh, &shead, sizeof (shead));
		read(fh, ptr, st.st_size);
		close(fh);

		menuitem = (void *) ptr;
		ptr += shead.menu * sizeof (struct smenuitem);
		sysvar = (void *) ptr;
		ptr += shead.key * sizeof (struct sdefine);
		sysconf_buf = (void *) ptr;
		ptr += shead.len;
		sysconf_menu = shead.menu;
		sysconf_key = shead.key;
		sysconf_len = shead.len;
		diff = sysconf_buf - shead.buf;
		for (n = 0; n < sysconf_menu; n++) {
			menuitem[n].name += diff;
			menuitem[n].desc += diff;
			menuitem[n].arg += diff;
		}
		for (n = 0; n < sysconf_key; n++) {
			sysvar[n].key += diff;
			sysvar[n].str += diff;
		}
	} else if (rebuild == NA) {
		rebuild = YEA;
		goto rebuild;
	}
}
