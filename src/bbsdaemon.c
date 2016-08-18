#include "webbs.h"

void
add_msg(int sig)
{
	int i;
	char buf[129], file[STRLEN], *id = currentuser.userid;

	/* monster: uses utime function instead of running external utilitie 'touch' */
	sethomefile(file, id, "wwwmsg.flush");
	utime(file, NULL);

	sethomefile(file, id, "msgfile");
	i = file_size(file) / sizeof(buf);
	if (get_record(file, &buf, sizeof(buf), i) == -1)
		return;

	buf[sizeof(buf) - 1] = '\0';
	sethomefile(file, id, "wwwmsg");
	append_record(file, buf, sizeof(buf));
}

void
abort_program(int sig)
{
	int stay = 0;
	struct userec *x;

	if (!strcmp(u_info->userid, currentuser.userid)) {
		stay = abs(time(0) - *(int *) (u_info->from + 32));
		memset(u_info, 0, sizeof (struct user_info));
	}
	if (stay > 7200)
		stay = 7200;
	x = getuser(currentuser.userid);
	if (x) {
		x->stay += stay;
		x->lastlogout = time(0);
		save_user_data(x);
	}
	exit(0);
}

int
daemon_main()
{
        int i, j, tsize;
        int utmpnum;
	struct userec *uid;

	signal(SIGALRM, SIG_DFL);
	alarm(0);
	clearup_stuff();
	shm_init();
	tsize = getdtablesize();
        for (i = 0; i < tsize; i++)
                close(i);
        for (i = 0; i < NSIG; i++)
                signal(i, SIG_IGN);

	setpgid(0, getpid());

        seteuid(BBSUID);
        if(my_geteuid() != BBSUID) return -1;
	chdir(BBSHOME);
        shm_init();
	utmpnum = getpid();

	for (j = 0; j < 5; j++) {
		for (i = 0; i < MAXACTIVE; i++) {
		        u_info = &(shm_utmp->uinfo[i]);
			if (u_info->pid == utmpnum) break;
		}
		if (i < MAXACTIVE) break;

		switch (j) {
			case 0:
				usleep(100);
				break;
			case 1:
				usleep(500);
				break;
			case 2:
				sleep(1);
				break;
			default:
				sleep(3);
		}
	}
	if (i >= MAXACTIVE) return -1;

	if ((uid = getuser(u_info->userid)) == 0) return -1;
	memcpy(&currentuser, uid, sizeof(struct userec));

#ifdef SETPROCTITLE
	char proctitle[40];
	snprintf(proctitle, sizeof(proctitle), "user %s from [%s]", currentuser.userid, currentuser.lasthost);
	setproctitle(proctitle);
#endif

        signal(SIGUSR2, add_msg);
        signal(SIGHUP, abort_program);
	nice(3);
        while (1) {
                sleep(60);
                if (abs(time(0) - u_info->idle_time) > 600) {
                        raise(SIGHUP);
                }
        }

	exit(0);
	return 0;
}
