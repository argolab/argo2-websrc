// add by Flier - 2000.5.12 - End

/* file.c */
#define trim(s) (ltrim(rtrim(s)))
#define file_size(x) f_stat(x)->st_size
#define file_time(x) f_stat(x)->st_mtime
#define file_rtime(x) f_stat(x)->st_atime
#define file_exist(x) (file_time(x)!=0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)

/* http.c */
#define setcookie(a, b) printf("<script>document.cookie='%s=%s'</script>\n", a, b)
#define redirect(x)     printf("<meta http-equiv='Refresh' content='0; url=%s'>\n", x)
#define refreshto(x, t) printf("<meta http-equiv='Refresh' content='%d; url=%s'>\n", t, x)
#define cgi_head()      printf("Content-type: text/html; charset=%s\n\n", CHARSET)

/* html.c */
#define article_read_foot()	form_read_foot()
#define mail_read_foot()	form_read_foot()


#define SHOW_IP(uentp) \
                    ((uentp->hideip == 'N') || \
                     (isfriend(uentp->userid) && (uentp->hideip == 'F')) || \
                      HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_SEEIP)) \


