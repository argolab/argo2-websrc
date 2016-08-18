#include "webbs.h"

int getGroupSet(void) {
	FILE *fp;
	int i, j, k;
	char *ptr, *ptr2;

	fp = fopen("etc/menu.ini", "r");
	if (fp == NULL) return -1;

	SECNUM = 0;
        for (i = 0; i < 36; i++) {
		seccode[i][0] = '\0';
		secname[i][0][0] = '\0';
		secname[i][1][0] = '\0';
        }

	j = 0; k = 0;
	while (1) {
		fgets(genbuf, sizeof(genbuf), fp);
		if (feof(fp)) break;
		if (strstr(genbuf, "@EGroups")) {
			if (genbuf[0] != '@') continue;
			ptr = strtok(genbuf, "\"");
			ptr = strtok(NULL, "\"");
			if (*ptr >= '0' && *ptr <= '9') i = *ptr - '0'; 
			else if (*ptr >= 'A' && *ptr <= 'Z') i = *ptr - 'A' + 10;
			else if (*ptr >= 'a' && *ptr <= 'z') i = *ptr - 'a' + 10;
			else continue;
			j ++;
                        ptr = strtok(NULL, ")");
                        ptr = strtok(NULL, "[");
                        ptr2 = ptr + strlen(ptr) - 1;
                        while (strchr("- ", *ptr2)) {
                                ptr2 --;
                        }
                        *(ptr2 + 1) = '\0';
			while (*ptr == ' ' && ptr < ptr2) ptr ++;
			strlcpy(secname[i][0], ptr, sizeof(secname[i][0]));
			ptr = strtok(NULL, "\"");
			ptr2 = ptr + strlen(ptr) - 1;
			while (*ptr2 != ']') ptr2--;
			*(ptr2 + 1) = '\0';
			snprintf(secname[i][1], sizeof(secname[i][1]), "[%s", ptr);
		}
		if (strstr(genbuf, "EGROUP")) {
			if (genbuf[0] != 'E') continue;
			ptr = strtok(genbuf, "\"");
			if (ptr[6] >= '0' && ptr[6] <= '9') i = ptr[6] - '0';
			else if (ptr[6] >= 'A' && ptr[6] <= 'Z') i = ptr[6] - 'A' + 10;
			else if (ptr[6] >= 'a' && ptr[6] <= 'z') i = ptr[6] - 'a' + 10;
			else continue;
			k++;
			ptr = strtok(NULL, "\"");
			strlcpy(seccode[i], ptr, sizeof(seccode[i]));
		}
	}
	SECNUM = j;
	if (k < SECNUM) SECNUM = k;
	return 0;
}
