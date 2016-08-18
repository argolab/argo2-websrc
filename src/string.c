#include "webbs.h"

int
ci_strncmp(s1, s2, n)
register char *s1, *s2;
register size_t n;
{       
        char c1, c2; 
        
        while (n-- > 0) {
                c1 = *s1++;
                c2 = *s2++;
                if (c1 >= 'a' && c1 <= 'z')
                        c1 += 'A' - 'a';
                if (c2 >= 'a' && c2 <= 'z')
                        c2 += 'A' - 'a';
                if (c1 != c2)
                        return (c1 - c2);
                if (c1 == 0)
                        return 0;
        }
        return 0;
}

int
strsncpy (s1, s2, n)
char *s1;
char *s2;
int n; {
        int l=strlen(s2);
        if(n<0) return -1;
        if(n>l+1) n=l+1;
        strncpy(s1, s2, n-1);
        s1[n-1]=0;
        return 0;
}

char *
strnncpy (s, l, s2)
char *s;
int *l;
char *s2; {
        if (strncpy((char *)(s+(*l)), s2, strlen(s2)) == NULL)
                return NULL;
        (*l)+=strlen(s2);
        return s;
}

char *
getdatestring (now)
time_t now;
{
        struct tm *tm;
        char weeknum[7][3]={"天","一","二","三","四","五","六"};

        tm = localtime((time_t *)&now);
        sprintf(datestring,"%4d年%02d月%02d日%02d:%02d:%02d 星期%2s",
                tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
                tm->tm_hour,tm->tm_min,tm->tm_sec,
                weeknum[tm->tm_wday]);
        return datestring;
}

char *
Ctime (t)
time_t t;
{
        static char s[80];
        sprintf(s, "%24.24s", ctime(&t));
        return (&s[0]);
}

char *
Cdtime (t)
time_t t;
{
        static char s[80];
        sprintf(s, "%24.24s", ctime(&t));
        return (&s[4]);
}

char *
inttostr (i)
int i;
{
        static char buf[20];		/* monster: 20 = floor(lg(2^64)) + 1, the function can processs 64-bit integers */

        sprintf(buf, "%d", i);
        return buf;
}
