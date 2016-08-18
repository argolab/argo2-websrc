#ifndef BIGGER_MOVIE
  #define MAXMOVIE                6       /* ��������� (�ޱ߿�) */
  #define MOVIE_HEIGHT 	80
#else
  #define MAXMOVIE                8       /* ��������� (�б߿�) */
  #define MOVIE_HEIGHT 	120
#endif

#define UPLOAD_MAX	(1024 * 1024)

#define FIRST_PAGE      "/main.html"
#define CSS_FILE        "/style.css"
#define CHARSET         "gb2312"
#define UCACHE_SHMKEY   get_shmkey("UCACHE_SHMKEY")
#define UTMP_SHMKEY     get_shmkey("UTMP_SHMKEY")
#define BCACHE_SHMKEY   get_shmkey("BCACHE_SHMKEY")
#define PATTERN_PATH	"pattern/"

#define CRLF              "\r\n"
