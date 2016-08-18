#include "webbs.h"

/* the following code is mainly borrowed from Apache 1.3 source */

void sig_alarm(int sig) {
	exit(1);
}

void sig_restart(int sig) {
	restart_pending = 1;
}

void sig_term(int sig) {
	shutdown_pending = 1;
}

void die_child(int sig) {
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
}

void reapchild(int sig) {
        int state, pid;

        while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}

void my_signal(int signo, void *func) {
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef  SA_INTERRUPT            /* SunOS */
    act.sa_flags |= SA_INTERRUPT;
#endif
    sigaction(signo, &act, &oact);
}

void set_signals() {
//	struct sigaction sa;

	my_signal(SIGSEGV, SIG_IGN);
	my_signal(SIGBUS, SIG_IGN);
	my_signal(SIGABRT, SIG_IGN);
	my_signal(SIGILL, SIG_IGN);
	my_signal(SIGINT, sig_term);
	my_signal(SIGPIPE, SIG_IGN);
	my_signal(SIGHUP, SIG_IGN);
	my_signal(SIGUSR1, SIG_IGN);
//	my_signal(SIGHUP, sig_restart);
//	my_signal(SIGUSR1, sig_restart);
	my_signal(SIGTERM, sig_term);

/*
	sigemptyset(&sa.sa_mask);
#if defined(SA_ONESHOT)
	sa.sa_flags = SA_ONESHOT;
#elif defined(SA_RESETHAND)
	sa.sa_flags = SA_RESETHAND;
#endif

	sa.sa_handler = sig_coredump;
	if (sigaction(SIGSEGV, &sa, NULL) < 0)
		error_log("sigaction(SIGSEGV) fail to execute");
	if (sigaction(SIGBUS, &sa, NULL) < 0)
		error_log("sigaction(SIGBUS) fail to execute");
	if (sigaction(SIGABRT, &sa, NULL) < 0)
		error_log("sigaction(SIGABRT) fail to execute");
	if (sigaction(SIGILL, &sa, NULL) < 0)
		error_log("sigaction(SIGILL) fail to execute");
	sa.sa_flags = 0;

	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) < 0)
		error_log("sigaction(SIGPIPE) fail to execute");

	sa.sa_handler = sig_term;
	if (sigaction(SIGTERM, &sa, NULL) < 0)
		error_log("sigaction(SIGTERM) fail to execute");
	if (sigaction(SIGINT, &sa, NULL) < 0)
		error_log("sigaction(SIGINT) fail to execute");

	// we want to ignore HUPs and USR1 while we're busy processing one 
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sa.sa_handler = sig_restart;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
		error_log("sigaction(SIGHUP) fail to execute");
	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		error_log("sigaction(SIGUSR1) fail to execute");
*/

}
