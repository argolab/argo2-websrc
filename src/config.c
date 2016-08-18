#include "webbs.h"

char *get_conf(const char *item) {
	FILE *fp;
	static char conf_buf[256];
	char *ptr;

	if (item == NULL) return NULL;

	fp = fopen("etc/httpd.conf", "r");
	if (fp == NULL) fatal("Can't read config file");
	while (1) {
		fgets(conf_buf, 255, fp);
		if (feof(fp)) break;
		ptr = (char *)strtok(conf_buf, " \t\n");
		if (ptr == NULL) continue;
		if (ptr[0] == '#') continue;
		if (!strcmp(ptr, item)) {
			ptr = (char *)strtok(NULL, " \t\n");
			fclose(fp);
			return ptr;
		}
	}
	fclose(fp);
	return NULL;
}

int conf_error(char *msg) {
	printf("Config file error: %s\n", msg);
	exit(-1);
}
