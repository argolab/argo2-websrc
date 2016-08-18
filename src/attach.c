#include "webbs.h"

#ifdef ATTACH_UPLOAD
char *boundary;
char *uploadtemp(char *extension) {
	static char fname[80];
	int fd;
	char *ptr;

	/* Henry: Simply skip other headers */
	while (1) {
		fgets(genbuf, sizeof(genbuf), stdin);
		ptr = strtok(genbuf, "\r\n");
		if (ptr == NULL) break;
	}

	snprintf(fname, sizeof(fname), "tmp/%d%d.%s", getpid(), getppid(), extension);
	fd = open(fname, O_WRONLY|O_CREAT|O_EXCL, 0644);
	if (fd == -1) {
		return NULL;
	}
	while (1) {
		fgets(genbuf, sizeof(genbuf), stdin);
		if (strstr(genbuf, boundary)) break;
		write(fd, genbuf, strlen(genbuf));
	}
	close(fd);
	/* the next boundary is in genbuf now */
	return fname;
}

void send_attach(char *str) {
	char *board;
	char buf[1024*128], *fname;
	int fd, len;
	struct stat st;
	struct attacheader ah;

	strtok(str, "/");
	board = strtok(NULL, "/");
	if (!board) http_msg(404);
	fname = strtok(NULL, "");
	snprintf(buf, sizeof(buf), "A.%d.A", atoi(fname));
	
	if (!getattach(board, buf, &ah)) http_msg(404);
	snprintf(buf, sizeof(buf), "attach/%s/%s", 
		board, ah.filename);

	if (lstat(buf, &st) == -1) http_msg(404);
	fd = open(buf, O_RDONLY);
	if (fd == -1) http_msg(404);
	http_msg(200);
	printf("Content-Length: %d\r\n", (int) st.st_size);

	char	*content_type = get_content_type(ah.filetype);
	printf("Content-Type: %s;\r\n", content_type);
	if ( !strncmp(content_type, "text", 4) || !strncmp(content_type, "image", 5) ) 
		printf("Content-Disposition: inline; filename=\"%s\"\r\n", ah.origname);
	else  
		printf("Content-Disposition: attachment; filename=\"%s\"\r\n", ah.origname);
	printf("\r\n");
	fflush(stdout);
	
	while (1) {
		len = read(fd, buf, sizeof(buf));
		if (len <= 0) break;
		write(1, buf, len);
	}
	close(fd);
}

int multipart_trans() {
	int n, len, fd, i;
	char *spool, *ptr, *boundary, *ptr2, *ptr3;
	char *label;
	char *attach_type = "multipart/form-data";

	if (strncmp(content_type, attach_type, strlen(attach_type)))
		http_fatal("assert error");

	http_parm_free();

	n = content_length;
	if (n > 5000000) n = 5000000;
	if (n > UPLOAD_MAX + 2048) {
		http_fatal("上传附件不能超过 %d 字节", UPLOAD_MAX);
	}
	
	spool = calloc(n + 1, 1);
	if (spool == NULL) http_fatal("memory overflow");
	n = fread(spool, 1, n, stdin);
	spool[n] = 0;
	//http_fatal("length = %d<br/><pre>%s</pre>", content_length, spool);
	strtok(content_type, "=");
	boundary = strtok(NULL, "\r\n");
	if (boundary == NULL) http_fatal("Bad Boundary in request");
	ptr = strtok(spool, "\r\n");
	while (1) {
		if (ptr == NULL) break;
		/* Henry: this should be the boundary */
		//if (strstr(ptr, boundary) == NULL) http_fatal("Bad format 1: %s", ptr);
		/* babydragonm: 当表单数据含"%"的时候, 边界可能是不完整的
		 * 		客户端的bug? ft一下 -_-|||*/
		
		/* Henry: check if it is bottom boundary */
		snprintf(genbuf, sizeof(genbuf), "%s--", boundary);
		if (strstr(ptr, genbuf) != NULL) break;

		ptr = strtok(NULL, "\r\n");
		if (ptr == NULL) break;
		/* Henry: this should be a disposition line */
		if (strcasestr(ptr, "Content-Disposition") == NULL)
			http_fatal("Bad format 2");

		if (strcasestr(ptr, "filename=") != NULL) {
			/* Henry: This is a upload file section */
			if (strcasestr(ptr, "filename=\"\"") != NULL) {
				ptr = strtok(NULL, "\r\n");
				/* freestyler: 没有附件时, IE, firefox会有一行Content-type, 而opera无此行 */
				if( strcasestr(ptr, "Content-type")!=NULL) 
					ptr = strtok(NULL, "\r\n");
				continue;	//empty file
			}

			/* Get the filename between two quotas */
			ptr2 = strstr(ptr, "filename=");
			if (ptr2 == NULL) http_fatal("Bad format 10");
			ptr2 += 10;
			i = 0;
			while (*(ptr2 + i) != '\"' && *(ptr2 + i) != '\0') i++;
			if (*(ptr2 + i) == '\0') http_fatal("Bad format 11");
			*(ptr2 + i) = '\0';
			ptr3 = strrchr(ptr2, '\\');
			if (ptr3 == NULL) ptr3 = ptr2;
			else ptr3 ++;
			parm_add("UPLOADFILE", ptr3);

			ptr = strtok(NULL, "\r\n");
			if (ptr == NULL) http_fatal("Bad format 3");
			if (strcasestr(ptr, "Content-Type") != NULL) {
				ptr = strtok(NULL, "\r\n");
				if (ptr == NULL) http_fatal("Bad format 4");
			}

			/* Henry: generate the filename */
			snprintf(genbuf, sizeof(genbuf), "tmp/%s", ptr3);

			fd = open(genbuf, O_WRONLY|O_CREAT, 0644);
			if (fd == -1) http_fatal("Create fail");

			len = 0;
			while (strncmp(ptr + len, boundary, strlen(boundary)))
				len++;
			len -= 3;
	
			write(fd, ptr, len);
			close(fd);

			ptr2 = ptr + len + 1;
			ptr = strtok(ptr2, "\r\n");
		} else {
			/* Henry: this is normal section */
			label = strchr(ptr, '\"');
			label ++;
			len = 0;
			while (*(label + len) != '\"') len ++;
			*(label + len) = '\0';

			len = 0;
			ptr = strtok(NULL, "\0");
			while (*ptr == '\r' || *ptr == '\n') ptr ++;
			if (!strncmp(ptr, "--", 2) && !strncmp(ptr + 2, 
			    boundary, strlen(boundary))) {
				ptr2 = ptr;
				ptr = strtok(ptr2, "\r\n");
				continue;
			}
			while (strncmp(ptr + len, boundary, strlen(boundary)))
				len ++;
			len -= 2;
			ptr2 = ptr + len;
			while (ptr[len-1] == '\r' || ptr[len-1] == '\n') len --;
			ptr[len] = '\0';
			//__unhcode(ptr);
			/*babydragon: 使用"multipart/form-data"传输数据的时候, 不需要解码*/
			parm_add(trim(label), ptr);
			ptr = strtok(ptr2, "\r\n");
		}
	}
	free(spool);
	return 0;
}

int cmpattfname(void *fname, void *ah) {
	if (!strcmp(((struct attacheader *)ah)->filename, (char *)fname)) return 1;
	return 0;
}

int cmpattrname(void *fname, void *ah) {
	if (!strcmp(((struct attacheader *)ah)->origname, (char *)fname)) return 1;
	return 0;
}

int getattach(char *board, char *fname, struct attacheader *ah) 
{
	char dirfile[STRLEN];

	snprintf(dirfile, sizeof(dirfile), "attach/%s/.DIR", board);
	return search_record(dirfile, ah, sizeof(*ah), cmpattfname, fname);
}

int getattachorig(char *board, char *fname, struct attacheader *ah)
{
	char dirfile[STRLEN];

	snprintf(dirfile, sizeof(dirfile), "attach/%s/.DIR", board);
	return search_record(dirfile, ah, sizeof(*ah), cmpattrname, fname);
}

void save_attach(char *board, int articleid)
{
	struct boardheader *brd;
	struct attacheader attach;
	struct stat st;
	char *uploadfname;
	char *ptr, fname[STRLEN], buf[STRLEN];

	/* Henry: 把附件移动到正确位置 */
	uploadfname = getparm("UPLOADFILE");
	if (strlen(uploadfname) > 0) {
		brd = getbcache(board);
		if (brd && (brd->flag & BRD_ATTACH) == 0)
			http_fatal("本版不允许上传附件");
		snprintf(buf, sizeof(buf), "attach/%s",
			board);
		if (lstat(buf, &st) == -1 && mkdir(buf, 0755) == -1)
			http_fatal("无法创建附件目录");

		snprintf(buf, sizeof(buf), "attach/%s/.DIR",
			board);
		ptr = strrchr(uploadfname, '.');
		strlcpy(attach.filetype, ptr + 1, sizeof(attach.filetype));

		strlcpy(attach.origname, uploadfname, sizeof(attach.origname));
		snprintf(attach.filename, FNAMELEN, "A.%d.A", articleid);
		attach.articleid = articleid;
		strlcpy(attach.board, board, BFNAMELEN);
		if (append_record(buf, &attach, sizeof(attach)) == -1)
			http_fatal("无法创建附件索引");
		snprintf(buf, sizeof(buf), "attach/%s/%s",
			board, attach.filename);
		snprintf(fname, sizeof(fname), "tmp/%s", uploadfname);
		if (f_mv(fname, buf) == -1) http_fatal("上传失败");
	}
}

#endif /* ATTACH_UPLOAD */

int
upload_manage(void) {
	int i, total, n;
	struct attacheader header;
	struct fileheader fh;
	char fname[80], bfname[80];
	char *board;
	struct boardheader *bptr;

	init_all();
	if (!loginok) http_fatal("您还没有登陆");

	board = getparm("board");
	bptr = getbcache(board);

	if (bptr == NULL) http_fatal("无效的版面名字");
	if (!has_read_perm(&currentuser, board)) http_fatal("无效的版面名字");
	if (!(bptr->flag & BRD_ATTACH)) http_fatal("本版不允许上传附件");
	if (!has_BM_perm(&currentuser, board)) http_fatal("您无权管理版面附件");
	
	sprintf(fname, "attach/%s/.DIR", board);

	total = file_size(fname) / sizeof(header);
	printf("<center><table>");
	printf("<tr><td>序号</td><td>附件</td><td>相关文章</td><td>作者</td><td>上传时间</td><td>删除</td></tr>");

	for (i = 1; i <= total; i++) {
		if (get_record(fname, &header, sizeof(header), i) == -1) break;
		printf("<tr>");
		printf("<td>%d</td>", i);
		printf("<td><a href=attach/%s/%d.%s>%s</a></td>",
			board, atoi(header.filename + 2), 
			header.filetype, header.origname);
		sprintf(bfname, "boards/%s/.DIR", header.board);
		header.filename[0] = 'M';
		n = search_record(bfname, &fh, sizeof(fh), cmpfilename, header.filename);
		if (n == 0) {
			printf("<td>deleted</td>");
		} else 
			printf("<td><a href=bbscon?board=%s&start=%d>%s</a></td>",
				board, n, nohtml(fh.title));
		if (n != 0) {
			printf("<td>%s</td>", userid_str(fh.owner));
		} else {
			printf("<td>unknown</td>");
		}
		header.filename[0] = 'A';
		sprintf(bfname, "attach/%s/%s", board, header.filename);
		printf("<td>%12.12s</td>", Cdtime(file_time(bfname)));
		printf("<td><a href=bbsattachdel?board=%s&start=%d>删除</a></td>", board, i);
	}
	printf("</table>");
	printf("<br>\n");
	printf("<a href=bbsedituploadext?board=%s>编辑上传文件扩展名列表</a>", board);
	return 0;
}

int delete_attach(void)
{
	int n, start;
	char fname[80], bfname[80];
	char *board;
	struct attacheader header;
	struct fileheader fh;
	struct boardheader *bptr;

	init_all();
	if (!loginok) http_fatal("您还没有登陆");

	board = getparm("board");
	bptr = getbcache(board); 

	if (bptr == NULL) http_fatal("无效的版面名字");
	if (!has_read_perm(&currentuser, board)) http_fatal("无效的版面名字");
	if (!(bptr->flag & BRD_ATTACH)) http_fatal("本版不允许上传附件");
	if (!has_BM_perm(&currentuser, board)) http_fatal("您无权管理版面附件");

	start = atoi(getparm("start"));
	snprintf(fname, sizeof(fname), "attach/%s/.DIR", board);

	if (get_record(fname, &header, sizeof(header), start) == -1) {
		http_fatal("无效的编号");
	}

	snprintf(bfname, sizeof(bfname), "attach/%s/%s", board, header.filename);
	if (unlink(bfname) == -1) {
		http_fatal("删除失败");
	}

	header.filename[0] = 'M';
	snprintf(bfname, sizeof(bfname), "boards/%s/.DIR", board);
	n = search_record(bfname, &fh, sizeof(fh), cmpfilename, header.filename);
	if (n != 0) {
		fh.flag &= ~FILE_ATTACHED;
		substitute_record(bfname, &fh, sizeof(fh), n);
	}

	if (delete_record(fname, sizeof(struct attacheader), start) == -1) {
		http_fatal("删除失败");
	}
	
	printf("删除成功。。。");
	printf("<br>");
	printf("<a href=bbsattachman?board=%s>返回管理页面</a>", board);

	return 0;
}

int edit_uploadext() {
	char *board;
	char file[80];

	init_all();
	board = getparm("board");
	if (strlen(board) == 0) http_fatal("文件不存在");
	if (!has_BM_perm(&currentuser, board)) http_fatal("您无权访问此页");
	snprintf(file, sizeof(file), "attach/%s/upload.ext", board);
	if (!file_exist(file)) http_fatal("文件不存在");
	hs_init(10);
	hs_setfile("bbsedituploadext.ptn");
	hs_assign("BOARD", board);
	hs_assign("FILENAME", file);
	hs_end();
	http_quit();
	return 0;
}

int modify_uploadext() {
	char *source, file[80], *board;
	int fd;

	init_all();
	board = getparm("board");
	if (strlen(board) == 0) http_fatal("文件不存在");
	if (!has_BM_perm(&currentuser, board)) http_fatal("您无权访问此页");
	source = getparm("source");
	snprintf(file, sizeof(file), "attach/%s/upload.ext", board);
	if (!file_exist(file)) http_fatal("文件不存在");
	if (strlen(source) == 0) http_fatal("请填写文件内容");

	fd = open(file, O_WRONLY);
	if (fd < 0) http_fatal("打开文件出错");
	truncate(file, 0);
	if (write(fd, source, strlen(source)) < 0) http_fatal("无法写入文件");
	close(fd);
	printf("%s 的上传扩展名列表更新成功", board);
	return 0;
}

#ifdef ATTACH_UPLOAD
void show_attach_link(char *board, char *attfname)
{
	char displaypath[STRLEN];
	struct attacheader ah;
	struct boardheader *bptr;

	bptr = getbcache(board);
	if (bptr && (bptr->flag & BRD_ATTACH)) {
		if (board[0] && getattach(board, attfname, &ah)) {
			snprintf(displaypath, sizeof(displaypath), "attach/%s/%d.%s", board, atoi(ah.filename + 2), ah.filetype);
			printf("<br>");
			if (is_picture(&ah)) {
				printf("<font color=000000>本文带有图片 %s</font><br>", ah.origname);
				printf("<a href=%s target=\"_blank\"><img src=%s onLoad=\"javascript:if(this.width>570)this.width=570;\" onMouseover=\"javascript:if(this.width>570)this.width=570;\"></a>", 
					displaypath, displaypath);
			} else {
				printf("<font color=000000>本文带有附件，请查毒后再打开</font>: ");
				printf("<a href=%s>%s</a>", displaypath,
					ah.origname);
			}
		}
	}
}
#endif
