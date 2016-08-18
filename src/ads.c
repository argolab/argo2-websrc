#include "webbs.h"

const char *ADS_FILE_PATH = "etc/ads";

struct Ads {
  char href[255], img_src[255];  /*�ٶ�URL���Ȳ�����255*/
};

/*��ȡ��ǰ�����Ϣ��д�뵽ҳ���javascript���Թ�����*/
int ads_management() {
  FILE *fp;
  struct Ads ads[10];  /*�ٶ���ͬʱ���ڳ���10�����*/
  int ads_cnt = 0, i;

  init_all();
  if(!HAS_PERM(PERM_SYSOP))
    http_fatal("�����ҳ��");
  
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

/*��HTTP�����ж�ȡ�޸ĺ�Ĺ����Ϣ�����µ��ļ�*/
int ads_update() {
  char href_str[2550], img_src_str[2550];
  struct Ads ads[10];  /*�ٶ���ͬʱ���ڳ���10�����*/
  int ads_cnt = 0, i;
  char *ptr;
  FILE *fp;

  init_all();
  if(!HAS_PERM(PERM_SYSOP))
    http_fatal("�����ҳ��");

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

  printf("<script>alert('�޸ĳɹ��������´��Զ�������ҳʱ��Ч');</script>");
  redirect("bbsadsmanagement");
}

/* ��¼�û����������Ϣ,���ض��򵽹�汾���URL */
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

