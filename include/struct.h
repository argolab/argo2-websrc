struct title_t {
        int num;  
        int BM; //0 don't display BM
        char format[10][255];
        char *board;
        int depth;
};

struct dir {
        char board[20];
        char userid[14];
        char showname[40];
        char exp[80];
        char type[30];
        int filename;
        int date;
        int level;
        int size;
        int live;
        int click;
        int active;
        int accessed;
};

struct attacheader {
        char filename[FNAMELEN];
        char board[BFNAMELEN];
        char filetype[10];
	char origname[30];
	int articleid;
	char desc[48];
};
