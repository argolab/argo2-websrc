#include "webbs.h"

const char *ADS_FILE_PATH = "etc/ads";

struct Ads {
  char href[255], img_src[255];  /*假定URL长度不超过255*/
};

/*读取当前广告信息并写入到页面的javascript中以供操作*/
int ads_management() {
  FILE *fp;
  struct Ads ads[10];  /*假定不同时存在超过10个广告*/
  int ads_cnt = 0, i;

  init_all();
  if(!HAS_PERM(PERM_SYSOP))
    http_fatal("错误的页面");
  
  if((fp = fopen(ADS_FILE_PATH, "r")) == NULL)
    http_fatal("can't open ads file");

  while(!feof(fp)) {
    char line[255];
    if (!fgets(line, sizeof(line), fp))
      break;
    char *ptr1 = strtok(line, ",");
    char *ptr2 = strtok(NULL, "\r\n");
    strncpy(ads[ads_cnt].href, ptr1, sizeof(ads[ads_cnt].href));
    strncpy(ads[ads_cnt].img_src, ptr2, sizeof(ads[ads_cnt].img_src));
    ads_cnt++;
  }
  fclose(fp);

  hs_init(2);
  hs_setfile("bbsadsmanagement.ptn");

  if (ads_cnt > 0) {
    hs_setloop("ads");
    for (i = 0; i < ads_cnt; i++) {
     hs_assign("HREF", ads[i].href);
     hs_assign("IMG_SRC", ads[i].img_src);
     hs_doloop("ads");
   }
  }

  hs_end();
  http_quit();
}

/*从HTTP请求中读取修改后的广告信息并更新到文件*/
int ads_update() {
  char href_str[2550], img_src_str[2550];
  struct Ads ads[10];  /*假定不同时存在超过10个广告*/
  int ads_cnt = 0, i;
  char *ptr;
  FILE *fp;

  init_all();
  if(!HAS_PERM(PERM_SYSOP))
    http_fatal("错误的页面");

  strsncpy(href_str, getparm("hrefs"), sizeof(href_str));
  strsncpy(img_src_str, getparm("img_srcs"), sizeof(img_src_str));

  ptr = strtok(href_str, ";");
  while (ptr != NULL) {
    strsncpy(ads[ads_cnt++].href, ptr, sizeof(ads[0].href));
    ptr = strtok(NULL, ";");
  }

  ptr = strtok(img_src_str, ";");
  for (i = 0; i < ads_cnt; i++) {
    strsncpy(ads[i].img_src, ptr, sizeof(ads[0].img_src));
    ptr = strtok(NULL, ";");
  }

  fp = fopen(ADS_FILE_PATH, "w");
  for (i = 0; i < ads_cnt; i++) {
    fprintf(fp, "%s,%s\n", ads[i].href, ads[i].img_src);
  }
  fclose(fp);

  printf("<script>alert('修改成功，将在下次自动生成首页时生效');</script>");
  redirect("bbsadsmanagement");
}

/* 记录用户点击广告的信息,并重定向到广告本身的URL */
int ads_click() {
  char filebuf[256];
  char timebuf[50];
  char urlbuf[256];
  time_t now;
  int ads_click_log_fd;

  init_all();

  now = time(NULL);
  strlcpy(timebuf, ctime(&now), sizeof(timebuf));
  timebuf[strlen(timebuf) - 1] = '\0';

  ads_click_log_fd = open("wwwlog/ads_click_log", O_WRONLY|O_CREAT|O_APPEND, 0644);
  if (ads_click_log_fd < 0) fatal("open(ads_click_log)");

  snprintf(urlbuf, sizeof(urlbuf), "%s", getparm("q"));
  snprintf(filebuf, sizeof(filebuf), "[%s] [client %s]: %s\n",
      timebuf,
      fromhost,
      urlbuf);
  write(ads_click_log_fd, filebuf, strlen(filebuf));

  redirect(urlbuf);
}

