/* -*- Mode: C++; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: nil -*-
 * Copyright (c) 1998-2000 Ludus Design enr.
 * All Rights Reserved.
 * Tous droits r�serv�s.
 */

#include <sys/stat.h>
#define __USE_GNU
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>

#include "find_file.h"

Find_file_entry::Find_file_entry(const char *n, bool f) {
  name[0] = 0;
  name[1023] = 0;
  date[0] = 0;
  date[1023] = 0;
  strncpy(name, n, 1023);
  size = 0;
  is_folder = f;
}

class Find_file_Unix: public Find_file {
private:
  glob_t globbuf;
  int count;
  struct stat sbuf;
public:
  Find_file_Unix(const char *n);
  virtual ~Find_file_Unix();
  virtual bool eof();
  virtual Find_file_entry get_next_entry();
};

Find_file *Find_file::New(const char *n) {
  return new Find_file_Unix(n);
}

void Find_file::get_current_directory(char *s) {
  if(getcwd(s, 1024) == NULL)
    s[0] = 0;
	//Remove slashes from end
	char *w=s+strlen(s);
	while(w>=s) {
		if(*w=='/')
			*w=0;
		else
			break;
		w--;
	}
}

Find_file_Unix::Find_file_Unix(const char *n) {
  count = -1;

  globbuf.gl_offs = 0;

  glob(n, 0, NULL, &globbuf);
}

Find_file_Unix::~Find_file_Unix() {
  globfree(&globbuf);
}

bool Find_file_Unix::eof() {
  return !(globbuf.gl_pathc-count);
}

Find_file_entry Find_file_Unix::get_next_entry() {
  struct stat sbuf;
  char* buf;
  bool isdir;

  if(count == -1)
    buf = "..";
  else
    buf = globbuf.gl_pathv[count];

  count++;

  if(stat(buf, &sbuf) == -1)
    isdir = false;
  else
    isdir = S_ISDIR(sbuf.st_mode);
  
  Find_file_entry f(basename(buf), isdir);
  
  f.size = sbuf.st_size;
  strncpy(f.date, ctime(&sbuf.st_mtime), 1023);
  
  return f;
}
