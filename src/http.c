#include "webbs.h"

char parm_name[256][80], *parm_val[256];
int parm_num=0;

char *
getsenv (s)
char *s; {
        char *t=getenv(s);
        if(t) return t;   
        return "";
}

void 
parm_add (name, val)
char *name;
char *val; {
        int len = strlen(val);
        if (parm_num >= 255) http_fatal("too many parms.");
        parm_val[parm_num] = calloc(len+1, 1);
        if(parm_val[parm_num]==0) http_fatal("memory overflow2 %d %d", len, parm_num);
        strsncpy(parm_name[parm_num], name, 78);
        strsncpy(parm_val[parm_num], val, len+1);
        parm_num++;
}

void
http_parm_free(void) {
	int i;
	for (i = 0; i < parm_num; i++) 
		free(parm_val[i]);
	parm_num = 0;
}

char *
getparm (var)
char *var; {
        int n;
        for(n=0; n<parm_num; n++)
                if(!strcasecmp(parm_name[n], var)) return parm_val[n];
        return "";
}

int 
set_my_cookie () {
        FILE *fp;
        char path[256], buf[256], buf1[256], buf2[256];
        int my_t_lines=20, my_link_mode=0, my_def_mode=0;
        sprintf(path, "home/%c/%s/.mywww", toupper((int)currentuser.userid[0]), currentuser.userid);
        fp=fopen(path, "r");
        if(fp) {
                while(1) {
                        if(fgets(buf, 80, fp)==0) break;
                        if(sscanf(buf, "%80s %80s", buf1, buf2)!=2) continue;
                        if(!strcmp(buf1, "t_lines")) my_t_lines=atoi(buf2);  
                        if(!strcmp(buf1, "link_mode")) my_link_mode=atoi(buf2);
                        if(!strcmp(buf1, "def_mode")) my_def_mode=atoi(buf2);  
                }
                fclose(fp);
                sprintf(buf, "%d", my_t_lines);
                setcookie("my_t_lines", buf);  
                sprintf(buf, "%d", my_link_mode);
                setcookie("my_link_mode", buf);
                sprintf(buf, "%d", my_def_mode);
                setcookie("my_def_mode", buf);
		return 0;
        }
	else
		return -1;
}

void
http_init () {
        char buf2[1024], *t2, *t3, *buf;
	int n;

#ifdef ATTACH_UPLOAD
	const char multipart_header[] = "multipart/form-data";

	if (content_type && !strncmp(content_type, multipart_header, 
	    sizeof(multipart_header) - 1)){
		multipart_trans();
	} else {
#endif
	        n=content_length;
       		if(n>5000000) n=5000000;

	        http_parm_free();

	        buf=calloc(n+1, 1);
	        if (buf== NULL) http_fatal("memory overflow");
	        n = fread(buf, 1, n, stdin);
	        buf[n]=0;
	        t2=strtok(buf, "&");
	        while(t2) {
	                t3=strchr(t2, '=');
                	if(t3!=0) {
                        	t3[0]=0;
	                        t3++;
	                        __unhcode(t3);
	                        parm_add(trim(t2), t3);
	                }
	                t2=strtok(0, "&");
	        }
		free(buf);
#ifdef ATTACH_UPLOAD
	}
#endif

        if (query_string != NULL) {
                strsncpy(buf2, query_string, 1024);
                t2=strtok(buf2, "&");
                while(t2) {
                        t3=strchr(t2, '=');
                        if(t3!=0) {
                                t3[0]=0;
                                t3++;
                                __unhcode(t3);
                                parm_add(trim(t2), t3);
                        }
                        t2=strtok(0, "&");
                }
        }

        if (http_cookie != NULL) {
                strsncpy(buf2, http_cookie, 1024);
                t2=strtok(buf2, ";");
                while(t2) {
                        t3=strchr(t2, '=');
                        if(t3!=0) {
                                t3[0]=0;
                                t3++;
                                parm_add(trim(t2), t3);
                        }
                        t2=strtok(0, ";");
                }
                free(http_cookie);
		http_cookie = 0;
        }
}

void
http_term() {
        http_quit();
}

void
http_quit () {
//        printf("\n</html>\n");
	http_parm_free();
	exit(0);
}

int
__to16 (c)
char c; {
        if(c>='a'&&c<='f') return c-'a'+10;
        else if(c>='A'&&c<='F') return c-'A'+10;
        else if(c>='0'&&c<='9') return c-'0';
        else return 0;
}

int
__unhcode (s)
char *s; {
        int m, n;
        for(m=0, n=0; s[m]!=0; m++, n++) {
                if(s[m]=='+') {
                        s[n]=' ';
                        continue;
                }
                if(s[m]=='%') {
                        s[n]=__to16(s[m+1])*16+__to16(s[m+2]);
                        m+=2;
                        continue;
                }
                s[n]=s[m];
        }
        s[n]=0;
        return 0;
}

