#!/var/bbs/local/bin/python
#
# Generate the static content of html page for index.html
# The content include top ten topic, board promotion, hot boards and recommend
# artical
# Written by henry, 2006/1/16
#            betterman, 2008/5/31

import cgi
import os
import re
import random
import string
import struct
import sys
import tinpy
import urllib

topten_format_str = "13s20s56sIi"
fileheader_format_str = "16s14s14s56sIIII12s"
output_path = "/var/bbs/htdocs"
bbs_path = "/var/bbs"
board_path = bbs_path + "/boards"
topten_filename = bbs_path + "/etc/posts/day.0"
goodbrd_filename = bbs_path + "/etc/sysgoodbrd"
hotbrd_filename = bbs_path + "/0Announce/bbslist/board2"
top5recommend_filename = bbs_path + "/boards/Recommend/.DIGEST"
ads_filename = bbs_path + "/etc/ads"
template_filename = bbs_path + "/pattern/index.template"

def GetTopTen():
  size = struct.calcsize(topten_format_str)
  fd = open(topten_filename, 'rb')
  data = fd.read()
  fd.close()
  fp, tp, num= 0, size, 0
  db = {}
  topten = []
  while tp <= len(data):
    if num >= 10:
      break
    author, board, title, date, number = struct.unpack(topten_format_str, data[fp:tp])
    author = author[:author.find("\0")]
    board =  board[:board.find("\0")]
    title = title[:title.find("\0")]
    title = title.decode('gbk', 'ignore')
    title = title.encode('gbk', 'ignore')
    cgi.escape(title)

    if os.path.isfile('/'.join([board_path, board, 'M.%u.A' % date])):
      if not db.has_key(board):
        db[board] = 1
        num = num + 1
        topten.append({'title': title, 'boardname': board,
            'filename': 'M.%u.A' % date})
    fp, tp = tp, tp + size
  return topten

def GetAllBrd():
  fd = open(hotbrd_filename, 'r')
  lines = fd.readlines()
  fd.close()
  allbrd = []
  for line in lines[1:]:
    tokens = line.split()
    try:
      allbrd.append([tokens[2], tokens[4]])
    except IndexError:
      continue
  return allbrd

def GetGoodBrd():
  goodbrd = []
  allbrd = GetAllBrd()
  allbrdDic = {}
  for brd in allbrd:
    allbrdDic[brd[0]] = brd[1]
  fd = open(goodbrd_filename, 'r')
  lines = fd.readlines()
  for board in lines[:10]:
    board = board.strip()
    if allbrdDic.has_key(board):
      goodbrd.append({'boardname': board, 'chinesename': allbrdDic[board]})
    else:
      goodbrd.append({'boardname': board, 'chinesename': board})
  fd.close()
  return goodbrd

def GetAds():
  fd = open(ads_filename, 'r')
  lines = fd.readlines()
  fd.close()
  ads = []
  for line in lines:
    ad = line.strip().split(",")
    if len(ad) == 2:
      ads.append({'href':'bbsadsclick?q=%s' % urllib.quote(ad[0]), 'img_src': ad[1]})
  return ads

## Added by betterman@2007
def GetHotBrd():
  allbrd = GetAllBrd()
  hotbrd = []
  num = min(len(allbrd), 10)
  db = {}
  c = 0
  while c < num:
    exceed = int(random.paretovariate(0.5)) % num
    if not db.has_key(exceed):
      db[exceed] = 1
      hotbrd.append({'boardname': allbrd[exceed][0],
          'chinesename': allbrd[exceed][1]})
      c = c + 1
  return hotbrd

def GetRecommendArtical():
    size = struct.calcsize(fileheader_format_str)
    fd = open(top5recommend_filename, 'rb')
    data = fd.read()
    fd.close()
    #sys.stdout = open('/dev/stdout', 'w')
    fp, tp, count, db = 0, size, 0, []
    recommendfive = []
    while tp <= len(data):
      filename, owner, realowner, title, flag, fsize, id, filetime, reserved = struct.unpack(fileheader_format_str, data[fp:tp])
      filename = filename[:filename.index('\0')]
      title = title[:title.index('\0')]
      db.append([filename, title])
      fp, tp = tp, tp + size

    db.reverse()
    for (filename, title) in db:
      fn = bbs_path + "/boards/Recommend/" + filename
      try:
        file_handle = open(fn, 'r')
        lines = file_handle.readlines()
        if len(lines) < 12:
          continue
        origin_board = re.sub('\x1b.*?m', '', lines[4]).split()[2]
        cgi.escape(origin_board)	    
        origin_filename = re.sub('\x1b.*?m', '', lines[5]).split()[2]
        cgi.escape(origin_filename)
        recommender = re.sub('\x1b.*?m', '', lines[6]).split()[2]
        cgi.escape(recommender)
        author = re.sub('\x1b.*?m', '', lines[7]).split()[2]
        cgi.escape(author)
        summary = ""
        for i in (9, 10, 11, 12):
          if lines[i].startswith("--"):
            break
          summary += string.strip(lines[i], '\n')
          summary = summary.lstrip()
        summary = re.sub('\x1b.*?m', '', summary)
        summary = re.sub('\xa1\xa1', '', summary)
        summary = re.sub('\r', '', summary);
        #cgi.escape(summary)
        gs = summary.decode('gbk', 'ignore')
        if len(gs) > 64:
          gs = gs[:64]
        summary = gs.encode('gbk', 'ignore')
        cgi.escape(summary)
        #summary = summary[:128]
        recommendfive.append({
            'origin_board': origin_board,
            'title': title,
            'filename': filename, 
            'origin_filename': origin_filename,
            'recommender': recommender,
            'author': author,
            'summary': summary})
        count += 1
        if count >= 5:
          break
      except IndexError:
        continue
      except:
        print "<span>error</span>"
        raise
      continue
    
    fd.close()
    return recommendfive
    
if __name__ == '__main__':
  if (len(sys.argv) > 1):
    output_path = sys.argv[1]
  os.chdir(output_path)
  output_filename = output_path + "/index.htm"

  try:
    os.system('cp ' + output_filename + " " + output_filename + ".old")
    sys.stdout = open(output_filename, 'w')
    print tinpy.build(open(template_filename).read(),
        topten = GetTopTen(), goodbrd = GetGoodBrd(), hotbrd = GetHotBrd(),
        recommendfive = GetRecommendArtical(), ads = GetAds())
        
  except:
    os.system('cp ' + output_filename + ".old" + " " + output_filename)
    

