#include "webbs.h"

#define DOCUMENT_ROOT "htdocs"

void clearup_stuff() {
	if (http_cookie != NULL) free(http_cookie);
	http_parm_free();
	release_shm();
}

char *gmt_time(time_t t) {
        time_t time = t;
	struct tm *mytm;
        static char returnstr[50];
	
        mytm = gmtime(&time);
	strftime(returnstr, sizeof(returnstr), "%a, %d %b %Y %X GMT", mytm);
        return returnstr;
}

int http_msg(int msgno) {
	do_log2(ACCESS_LOG, msgno, 0);

        printf("%s %d %s\r\n", SERVER_PROTOCOL, msgno, msg_str(msgno)); /* status line */
        printf("Server: %s\r\n", HTTPD_VERSION);
        printf("Date: %s\r\n", gmt_time(time(NULL)));
	if(last_modified[0] != '\0')
		printf("Last-Modified: %s\r\n", last_modified);
	if (msgno >= 400 || msgno < 200) {
	        printf("Content-Type: text/html\r\n");
	        printf("\r\n");
		printf("<html><title>%s</title>", msg_str(msgno));
		printf("<body><h1>ERROR CODE: %d</h1></body></html>", 
		    msgno);
		fflush(stdout);
		exit(-1);
	} else if (msgno >= 300 && msgno < 400) {
		printf("\r\n");
		fflush(stdout);
		exit(-1);
	}
	fflush(stdout);
	return 0;
}

int header_request_handler() {
	char *ptr;
	static char url[512];
	
	fgets(url, sizeof(url) - 1, stdin); /* request line */

	if (strstr(url, "..")) http_msg(403);//·ÀÖ¹Â©¶´¹¥»÷

	request_method = (char *)strtok(url, " \r\n");
	if (request_method == NULL) {
		return http_msg(400);	//Bad Request
	}
	if (!strcmp(request_method, "BBS")) {
		bbs_daemon();		//BBS extended method
		return 0;
	}

	if (strcmp(request_method, "GET") && strcmp(request_method, "HEAD")
	    && strcmp(request_method, "POST")) {
		return http_msg(501);	//Not Implemented
	}

	ptr = (char *)strtok(0, " \r\n");
	if (ptr == NULL || ptr[0] != '/') {
		return http_msg(400);	//Bad Request
	}

	stuff_ent = (char *) strtok(ptr, "?");
	query_string = (char *) strtok(0, "\0");

	stuff_ent ++;	//skip the leading '/'

	http_cookie = NULL;
	atexit(clearup_stuff);
	char param_string[4096];
	
	while (fgets(param_string, sizeof(param_string), stdin) != 0) {
//Debug:	report(param_string); 
		ptr = (char *)strtok(param_string, "\r\n");
		if (ptr == NULL || strlen(ptr) == 0) break;
		header_param_handler(param_string);
	}

	if (strcmp(request_method, "POST")) {
		content_length = 0;
	}
	return 0;
}

int file_modified(time_t mtime) {
	strlcpy(last_modified, gmt_time(mtime), sizeof(last_modified));
	if(strcmp(last_modified, if_modified_since))
		return 1;
	else 
		return 0;
}

int show_file(char *filename) {
	char filebuf[1024], realname[MAXPATHLEN];
	int len, fd;
	struct stat fs;
	off_t left;	/* number of bytes needs to write */
	
	if (stat(filename, &fs) < 0) http_msg(404);	//Not Found
	if (!S_ISREG(fs.st_mode)) http_msg(403); 	/* forbidden */

	if (realpath(filename, realname) == NULL ||
	    strncmp(realname, BBSHOME"/htdocs/", strlen(BBSHOME) + 7) != 0)
		http_msg(404);

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		do_log2(ERROR_LOG, ELOG_ERROR, "open() of \"%s\" failed",
		    filename);
		http_msg(500);	//internal server error
	}
	if (file_modified(fs.st_mtime) == 0) {
		http_msg(304);  /* not modified */
	}

	if (range_start != 0 || range_end != 0) {
		http_msg(206);
	} else {
		http_msg(200);
	}
	

	/*
	ptr = filename + strlen(filename) - 1;
	while (ptr >= 0 && *ptr != '/') ptr --;
	ptr ++;
	*/
//        printf("Content-Type: text/html\r\n");

	if (range_start != 0)
		lseek(fd, range_start, SEEK_SET);
	
	if (range_end <= 0 || range_end > fs.st_size - 1) 
		range_end = fs.st_size -1;
	
	left = range_end - range_start + 1;
	printf("Content-Length: %ld\r\n", (long)left);
	
	if (range_start != 0 && range_end != fs.st_size -1) {
		printf("Accept-Ranges: bytes\r\nContent-Range: bytes %d-%d/%d\r\n",
		       range_start, range_end, fs.st_size);
	}
	
	printf("\r\n");
	fflush(stdout);

	if (strcmp(request_method, "HEAD")) {
		do {
			len = read(fd, filebuf, sizeof(filebuf));
			if (len <= 0) break;
			if (len < left )
				write(1, filebuf, len);
			else
				write(1, filebuf, left);
			left -= len;
		} while (left > 0 && len == sizeof(filebuf));
	}

	close(fd);
	return 0;
}

void doc_handler() {
	char buf[256];
	if (urldecode(buf, stuff_ent, sizeof(buf)) == -1) {
		http_fatal("unicode½âÂë´íÎó");
	}
	sprintf(genbuf, DOCUMENT_ROOT"/%s", buf);
	if (strlen(stuff_ent) == 0) {
		strcat(genbuf, "index.htm");
	}

	if (access(genbuf, R_OK) < 0) http_msg(404);

	show_file(genbuf);
}


void init_header() {
	last_modified[0] = '\0';
	if_modified_since[0] = '\0';
	range_start = 0;
	range_end = 0;
}

int header_param_handler(char *param_string) {
	char *ptr, *ptr2;

	//report(param_string); 
	ptr = (char *)strtok(param_string, " :");
	if (ptr == NULL) return -1;	//drop the incorrected parametre
	ptr2 = (char *)strtok(NULL, "\r\n");
	if (ptr2 == NULL) return -1;
	while (*ptr2 == ' ') ptr2 ++;
	if (ptr2 == '\0') return -1;
	if (!strcasecmp(ptr, "Content-Length")) {
		content_length = atoi(ptr2);
	} else
	if (!strcasecmp(ptr, "Content-Type")) {
		strlcpy(content_type, ptr2, sizeof(content_type));
	} else
	if (!strcasecmp(ptr, "Cookie")) {
		http_cookie = malloc(1024);
		if (http_cookie) {
			strlcpy(http_cookie, ptr2, 1024);
		} else {
			http_msg(500);
		}
	} else if (!strcasecmp(ptr, "Range")) {
		char *ptr3;
		if (!strncmp(ptr2, "bytes=", 6))
			ptr3 = ptr2 + 6;
		else
			return -1;
		
		while (*ptr3 != '-' && *ptr3 != '\0')
			ptr3++;
		
		if (*ptr3 != '-')  /* no minus found */
			return -1;
		ptr3++;
		range_start = atoi(ptr2 + 6);
		if (ptr3 != '\0')
			range_end = atoi(ptr3);
	} else if (!strcasecmp(ptr, "Host")) {
		strlcpy(host_header, ptr2, sizeof(host_header));
	} else if (!strcasecmp(ptr, "X-Real-IP")) { 
		if (!strcmp(fromhost, "127.0.0.1") )	 { // nginx proxy
			const char *remote_addr = NULL;
			if (strncmp(ptr2, "::ffff:", 7) == 0) remote_addr = ptr2 + 7;
			else remote_addr = ptr2;
			strncpy(fromhost, remote_addr, sizeof(fromhost));
			fromhost[23] = '\0';
		}
	}

	
	/* babydragon: 2008-03-28 
	 * we don't support keep-alive any more
	*/
	/* Henry 2004.8.22: We don't want to close this connection
	 * * after 30 secs, just close it after we send out the result
	 * of webpage*/
	/*
	if (!strcasecmp(ptr, "Connection") && !strcasecmp(ptr2, "Keep-Alive")) {
		alarm(KEEPALIVE_TIMEOUT);
	}*/

	/* babydragon: return 302 if not modified */

	if (!strcasecmp(ptr, "If-Modified-Since")) {
		strlcpy(if_modified_since, ptr2, sizeof(if_modified_since));
	}
	return 0;
}

int process() {
	signal(SIGALRM, sig_alarm);
	alarm(BROWSER_TIMEOUT);
	init_header();
	header_request_handler();

	if (!strncmp(stuff_ent, "bbs", 3)) {
		do_stuff();
  }
#ifdef ATTACH_UPLOAD
	else if (!strncmp(stuff_ent, "attach/", 7))
		send_attach(stuff_ent);
#endif
	else
		doc_handler();

	close(1);

	return 0;
}

void bbs_daemon() {
	pid_t pid;

	close(0);
	close(2);
	pid = fork();
	//log_limit("bbs_daemon");
	if (pid < 0) {
		pid = 0;
		send(1, &pid, sizeof(pid_t), 0);
		do_log2(HTTPD_LOG, 0, "BBS daemon failed to start");
		exit(1);
	}
	if (pid > 0) {
		send(1, &pid, sizeof(pid_t), 0);
		close(1);
		exit(0);
	}
	if (daemon_main() == -1) {
		do_log2(HTTPD_LOG, 0, "Unexpected error when starting daemon");
		exit(1);
	}
}

char *msg_str(int errcode) {
	switch (errcode) {
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 204: return "No Content";
	case 206: return "Partial Content";
	case 301: return "Moved Permanently";
	case 302: return "Moved Temporarily";
	case 304: return "Not Modified";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 416: return "Requested Range Not Satisfiable";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	default:
		return "";
	}
}
