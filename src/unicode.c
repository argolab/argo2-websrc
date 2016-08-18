#include "webbs.h"

int urldecode(char *dst, const char *src, int bufflen) {
	int len = 0, byte, i;
	char ch;
	while (*src) {
		if (*src == '%') {
			byte = 0;
			for (i = 0; i < 2; i++) {
				src++;
				ch = *src;
				if (ch == '\0') return -1;
				if (ch >= '0' && ch <= '9')
					byte = byte * 16 + (int) (ch - '0');
				else if (ch >= 'a' && ch <= 'f')
					byte = byte * 16 + (int) (ch - 'a') + 10;
				else if (ch >= 'A' && ch <= 'F')
					byte = byte * 16 + (int) (ch - 'A') + 10;
				else return -1;
			}
			dst[len++] = (char) byte;
			src++;
		} else {
			dst[len++] = *src;
			src++;
		}
		if (len >= bufflen) return -1;
	}
	dst[len] = '\0';
	return len;
}
