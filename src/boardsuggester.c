#include "webbs.h"

int m_board_suggester() {
	char board[BFNAMELEN + 1];
	int len = 0;
	int i;

	init_all();

	strsncpy(board, getparm("boardname"), BFNAMELEN);
	len = strlen(board);

	if (len == 0) {
		printf("<?xml version=\"1.0\"?>\n"
		       "<boards></boards>\n\n");
		http_quit();
		return 0;
	} else {
		printf("<?xml version=\"1.0\"?>\n");
		printf("<boards>\n");
		for (i = 0; i < MAXBOARD; i++) {
                        if (shm_bcache->bcache[i].filename[0]<=32 || 
			    shm_bcache->bcache[i].filename[0]>'z') 
				continue;
                        if (!has_read_perm(&currentuser, 
			    shm_bcache->bcache[i].filename)) 
				continue;
                        if (shm_bcache->bcache[i].parent != 0)
				continue;
			if (ci_strncmp(board, shm_bcache->bcache[i].filename, len) == 0) {
				printf("<board>%s</board>\n", shm_bcache->bcache[i].filename);
			}
		}
		printf("</boards>\n\n");
		http_quit();
		return 0;
	}

	

}
