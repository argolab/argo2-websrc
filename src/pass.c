#include "webbs.h"

typedef struct {
	unsigned long A, B, C, D;
	unsigned long Nl, Nh;
	unsigned long data[16];
	int num;
} MD5_CTX;


void MD5Init(MD5_CTX *);
void MD5Update(MD5_CTX *, const unsigned char *, unsigned int);
void MD5Final(MD5_CTX *, unsigned char[16]);

void
igenpass (passwd, userid, md5passwd)
const char *passwd;
const char *userid;
unsigned char md5passwd[];
{
	static const char passmagic[] =
		" #r3:`>/CH'M&p%<xCj?bqd=/?L7o:N.s;j}Ouo!--PhX j^icU3aX{]?7`<(jOt";

	MD5_CTX md5;

	MD5Init(&md5);

	/* update size > 128 */
	MD5Update(&md5, (unsigned char *) passmagic, 64);
	MD5Update(&md5, (unsigned char *) passwd, strlen(passwd));
	MD5Update(&md5, (unsigned char *) passmagic, 64);
	MD5Update(&md5, (unsigned char *) userid, strlen(userid));

	MD5Final(&md5, md5passwd);
	md5passwd[0] = 0;
}

int
setpasswd (passwd, user)
const char *passwd;
struct userec *user;
{
	igenpass(passwd, user->userid, user->passwd);
	return 1;
}

void
genpasswd (passwd, md5passwd)
const char *passwd;
unsigned char md5passwd[];
{
	igenpass(passwd, "BBS System", md5passwd);
}

int
checkpasswd (passwd, test)
const char *passwd;
const char *test;
{
	static char pwbuf[DES_PASSLEN];
	char *pw;

	strlcpy(pwbuf, test, DES_PASSLEN);
	pw = crypt_des(pwbuf, (char *) passwd);
	return (!strcmp(pw, passwd));
}

int
checkpasswd2 (passwd, user)
const char *passwd;
const struct userec *user;
{
	unsigned char md5passwd[MD5_PASSLEN];

	if (user->passwd[0]) {
		if (checkpasswd(user->passwd, passwd)) {
			setpasswd(passwd, (struct userec *) user);
			return YEA;
		}
		return NA;
	} else {
		igenpass(passwd, user->userid, md5passwd);
		return !memcmp(md5passwd, user->passwd, MD5_PASSLEN);
	}
}

int
checkpasswd3 (passwd, test)
const char *passwd;
const char *test;
{
	unsigned char md5passwd[MD5_PASSLEN];

	igenpass(test, "BBS System", md5passwd);
	return !memcmp(md5passwd, passwd, MD5_PASSLEN);
}
