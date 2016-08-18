#include "webbs.h"

static int access_log_fd, error_log_fd, httpd_log_fd;

int fatal(char *msg) {
        perror(msg);
        exit(1);
}

void prepare_log() {
	struct tm *mytm;
	time_t now;
	char access_fname[80];

	now = time(NULL);
	mytm = localtime(&now);
	strftime(access_fname, sizeof(access_fname), "wwwlog/access_log.%y%m%d", mytm);
	access_log_fd = open(access_fname, O_WRONLY|O_CREAT|O_APPEND, 0644);
//	access_log_fd = open("wwwlog/access_log", O_WRONLY|O_CREAT|O_APPEND, 0644);
	if (access_log_fd < 0) fatal("open(access_log)");
	httpd_log_fd = open("wwwlog/httpd_log", O_WRONLY|O_CREAT|O_APPEND, 0644);
	if (httpd_log_fd < 0) fatal("open(httpd_log)");
	error_log_fd = open("wwwlog/error_log", O_WRONLY|O_CREAT|O_APPEND, 0644);
	if (error_log_fd < 0) fatal("open(error_log)");
}

void close_log() {
	if (access_log_fd >= 0) close(access_log_fd);
	if (httpd_log_fd >= 0) close(httpd_log_fd);
	if (error_log_fd >= 0) close(error_log_fd);
	access_log_fd = httpd_log_fd = error_log_fd = -1;
}

void do_log2(int log_type, int log_param, char *fmt, ...) {
/* log_type:
 *   ACCESS_LOG: log_param = http message number defined in RFC1945
 *               fmt = NULL
 *   HTTPD_LOG:  fmt = string to log into httpd_log file
 *   ERROR_LOG:  log_param = ELOG_WARNING | ELOG_ERROR | ELOG_FATAL
 *               fmt = string to log into error_log file
 */
	va_list ap;
	static char logbuf[256], filebuf[256];
	char timebuf[50], filename[80];
	time_t now;
	extern int errno;
	struct tm *mytm;

	if (fmt) {
		va_start(ap, fmt);
		vsprintf(logbuf, fmt, ap);
		va_end(ap);
	}

	now = time(NULL);
	strlcpy(timebuf, ctime(&now), sizeof(timebuf));
	timebuf[strlen(timebuf) - 1] = '\0';

	switch (log_type) {
	case ACCESS_LOG:
		if (stuff_ent == NULL || strstr(stuff_ent, "bbs") == NULL ||
				strstr(stuff_ent, "bbsgetmsg") ||
				strstr(stuff_ent, "bbsfoot") 
				) 
			return;	//Henry: only scripts are logged
		mytm = localtime(&now);
		strftime(filename, sizeof(filename), "wwwlog/access_log.%y%m%d", mytm);
//		if (!file_exist(filename)) {
//			close(access_log_fd);
//			access_log_fd = open(filename, O_WRONLY|O_CREAT, 0644);
//			if (access_log_fd < 0) fatal("open(access_log)");
//		}
//		if (stuff_ent == NULL || strcasestr(stuff_ent, ".jpg") || strcasestr(stuff_ent, ".gif") 
//		    || strcasestr(stuff_ent, ".jpeg") || strcasestr(stuff_ent, ".js")
//		    || strcasestr(stuff_ent, ".css") || strcasestr(stuff_ent, ".wav"))
//			return;		//Henry: We don't expect to log down image files

		snprintf(filebuf, sizeof(filebuf), "[%s][%s][%u] \"%s /%s%s%s\" %d\n",
		    timebuf,
		    fromhost,
		    getpid(),
		    request_method?request_method:"",
		    stuff_ent?stuff_ent:"",
		    query_string?"?":"",
		    query_string?query_string:"",
		    log_param);
		file_append(filename, filebuf);
//		write(access_log_fd, filebuf, strlen(filebuf));
		break;
	case HTTPD_LOG:
		snprintf(filebuf, sizeof(filebuf), "[%s] httpd(pid = %d): %s\n",
		    timebuf,
		    getpid(),
		    logbuf);
		write(httpd_log_fd, filebuf, strlen(filebuf));
		break;
	case ERROR_LOG:
		snprintf(filebuf, sizeof(filebuf), "[%s] [%s] [client %s] %s: %s\n",
		    timebuf,
		    (log_param == ELOG_WARNING) ? "warning" :
		    ((log_param == ELOG_FATAL) ? "fatal" : "error"),
		    fromhost,
		    strerror(errno),
		    logbuf);
		write(error_log_fd, filebuf, strlen(filebuf));
	}
}
