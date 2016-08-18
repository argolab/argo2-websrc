#include "webbs.h"

char hs_genbuf[8][128];

struct hs_struct {
	char *pattern;
	char *rword;
};
static struct hs_struct *hs_list = NULL;
static struct hs_struct *hs_clistptr;
static struct hs_struct hs_constlist[] = {
	{"BBSHOST", BBSHOST},
	{"BBSNAME", BBSNAME},
	{NULL, NULL}
};
static int hs_sum = 0, hs_entry = 0, hs_loopos;
static FILE *hs_fp;
static char hs_buf[256];


void hs_init(int unit) {
	hs_fp = NULL;
	hs_sum = unit;
	hs_entry = 0;
	hs_list = (struct hs_struct *) malloc(sizeof(struct hs_struct) * unit);
}

char *safe_strcat(char *str1, const char *str2, int catlen, int *len) 
{
	if (catlen == 0) catlen = strlen(str2);
	if (catlen > *len) {
		len = 0;
		return NULL;
	}
	*len -= catlen;
	strncat(str1, str2, catlen);
	return str1;
}

void hs_assign(char *pat, char *word) {
	int i;
	for (i = 0; i < hs_entry; i++)
		if (!strcmp(pat, hs_list[i].pattern)) {
			hs_list[i].rword = word;
			return;
		}

	if (hs_entry == hs_sum) return;
	hs_list[hs_entry].pattern = pat;
	hs_list[hs_entry].rword = word;
	hs_entry ++;
}

static void hs_translate(char *line) {
	int i;
	char *ptr, *head, *curr;
	static char buf[4096];
	static int condition = 1;
/*
	if (hs_entry == 0 || hs_list == 0) {
		printf("%s", line);
		return;
	}
*/
	if (ptr = strstr(line, "<!-- IF")) {
		ptr += 7;
		curr = strtok(ptr, " \t\n");
		condition = 0;
		for (i = 0; i < hs_entry; i++)
			if (!strncmp(curr, hs_list[i].pattern, strlen(hs_list[i].pattern))) {
				condition = 1;
				break;
			}
		return;
	}

	if (strstr(line, "<!-- ENDIF -->")) {
		condition = 1;
		return;
	}

	if (!condition) return;

	if (ptr = strstr(line, "<!-- SHOWFILE")) {
		ptr += 13;
		curr = strtok(ptr, " \t\n");
		for (i = 0; i < hs_entry; i++)
			if (!strncmp(curr, hs_list[i].pattern, strlen(hs_list[i].pattern))) {
				disp_file(hs_list[i].rword);
				break;
			}
		return;
	}

	buf[0] = 0;
	head = line;
	curr = line;
	while (*curr != '\0') {
		if (*curr == '{') {
			curr ++;

			for (hs_clistptr = hs_constlist; hs_clistptr->pattern; hs_clistptr ++)
				if (!strncmp(curr, hs_clistptr->pattern,
				    strlen(hs_clistptr->pattern)) &&
				    *(curr + strlen(hs_clistptr->pattern)) == '}') {
					strncat(buf, head, (int) (curr - head - 1));
					strcat(buf, hs_clistptr->rword);
					curr += strlen(hs_clistptr->pattern) + 1;
					head = curr;
					break;
				}

			for (i = 0; i < hs_entry; i++)
				if (!strncmp(curr, hs_list[i].pattern, 
				    strlen(hs_list[i].pattern)) && 
				    *(curr + strlen(hs_list[i].pattern)) == '}') {
					strncat(buf, head, (int) (curr - head - 1));
					strcat(buf, hs_list[i].rword);
					curr += strlen(hs_list[i].pattern) + 1;
					head = curr;
					break;
				}
		} else 
			curr ++;
	}
	strcat(buf, head);
	printf("%s", buf);
}

int hs_setfile(char *filename) {
	char fpath[256];
	snprintf(fpath, sizeof(fpath), "%s%s", PATTERN_PATH, filename);
	hs_fp = fopen(fpath, "r");
	return (hs_fp == NULL) ? -1 : 0;
}

void hs_setloop(char *tag) {
	char btag[32];

	snprintf(btag, sizeof(btag), "BEGIN %s", tag);
	while (fgets(hs_buf, sizeof(hs_buf), hs_fp) != NULL) {
		if (strstr(hs_buf, btag) == NULL)
			hs_translate(hs_buf);
		else {
			hs_loopos = ftell(hs_fp);
			return;
		}
	}
}

void hs_doloop(char *tag) {
	char etag[32];

	fseek(hs_fp, hs_loopos, SEEK_SET);
	snprintf(etag, sizeof(etag), "END %s", tag);
	while (fgets(hs_buf, sizeof(hs_buf), hs_fp) != NULL)
		if (strstr(hs_buf, etag) == NULL)
			hs_translate(hs_buf);
		else
			break;
}

void hs_end() {
	if (hs_fp) {
		while (fgets(hs_buf, sizeof(hs_buf), hs_fp))
			hs_translate(hs_buf);
		fclose(hs_fp);
	}
	free(hs_list);
	hs_sum = 0;
	hs_entry = 0;
}
