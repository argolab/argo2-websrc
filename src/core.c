#include "webbs.h"

int main(int argc, char *argv[], char *envp[]) {
	int sockfd, accefd, rsinlen, on = 1, max_conn;
	pid_t pid;
	struct sockaddr_in6 sin, rsin;
	unsigned long waittime;

	if (argc < 2 || (server_port = atoi(argv[1])) == 0) return -1;
	waittime = 1024 * 16;	/* in usec */
	chdir(BBSHOME);
	signal(SIGCHLD, reapchild);

	set_signals();

        if (fork()) exit(0);
        setsid();
        if (fork()) exit(0);
        umask(0);

	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fatal("sockfd");
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	memset(&sin, 0, sizeof(struct sockaddr));
	sin.sin6_family = AF_INET6;
	sin.sin6_port = htons(server_port);
	sin.sin6_addr = in6addr_any;
	if (bind(sockfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in6)) < 0){
		fatal("bind");
	}
#define MAX_CONNECT 256
	max_conn = MAX_CONNECT;
	if (listen(sockfd, max_conn) < 0) {
		fatal("listen");
	}

#ifdef SET_EFFECTIVE_ID
	if (getuid() == 0) {
		setegid((gid_t) BBSGID);
		if (getegid() != (gid_t) BBSGID) return -1;
		seteuid((uid_t) BBSUID);
		if (geteuid() != (uid_t) BBSUID) return -1;
	} else {
		setgid((gid_t) BBSGID);
		if (getgid() != (gid_t) BBSGID) return -1;
		setuid((uid_t) BBSUID);
		if (getuid() != (uid_t) BBSUID) return -1;
	}
#else
	setgid((gid_t) BBSGID);
	if (getgid() != (gid_t) BBSGID) return -1;
	setuid((uid_t) BBSUID);
	if (getuid() != (uid_t) BBSUID) return -1;
#endif

	prepare_log();
	do_log2(HTTPD_LOG, 0, "httpd started at port %d", server_port);

#ifdef SETPROCTITLE
	char proctitle[64];
	init_setproctitle(argc, argv, envp);
	snprintf(proctitle, sizeof(proctitle), "httpd: accepting connections (port %d)", server_port);
	setproctitle(proctitle);
#endif

	nice(5);
	char pidfile[30];
	char pidstr[10];
	sprintf(pidstr, "%d", getpid());
	sprintf(pidfile, "wwwlog/httpd.%d.pid", server_port);
	unlink(pidfile);
	file_append(pidfile, pidstr);
	//babydragon: sort cmdlist for bin-search
	cmd_sort();
	while (1) {
		memset(&rsin, 0, sizeof(rsin));
		rsinlen = sizeof(rsin);
		accefd = accept(sockfd, (struct sockaddr *)&rsin, &rsinlen);
		if (accefd >= 0) {
			pid = fork();
			if (pid < 0) {
				do_log2(ERROR_LOG, ELOG_ERROR, "fork()");
				close(accefd);
				break;
			}
			if (pid == 0) {
				inet_ntop(AF_INET6, &rsin.sin6_addr, genbuf, sizeof(genbuf));
				if (strncmp(genbuf, "::ffff:", 7) == 0)
					memmove(genbuf, genbuf + 7, sizeof(genbuf) - 7);
				strncpy(fromhost, genbuf, sizeof(fromhost));
				fromhost[23] = '\0';

				close(sockfd);
				dup2(accefd, 0);
				dup2(accefd, 1);
				dup2(accefd, 2);
				if (accefd > 2) close(accefd);
			        signal(SIGSEGV, SIG_DFL);
			        signal(SIGBUS, SIG_DFL);
			        signal(SIGABRT, SIG_DFL);
			        signal(SIGILL, SIG_DFL);
			        signal(SIGPIPE, SIG_DFL);
				signal(SIGHUP, SIG_DFL);
				signal(SIGTERM, die_child);
				signal(SIGUSR1, die_child);
				nice(3);
#ifdef SETPROCTITLE
				snprintf(proctitle, sizeof(proctitle), "Connection established from %s (port %d)", fromhost, server_port);
				setproctitle(proctitle);
#endif
				process();
				close(accefd);
				exit(0);
			} else {
				close(accefd);
			}
		} else {
			//received a signal?
			if (restart_pending) {
			} else if (shutdown_pending) {
				break;
			}
		}
	}
	do_log2(HTTPD_LOG, 0, "httpd ended at port %d", server_port);
	close(sockfd);
	close_log();
	return 0;
}
