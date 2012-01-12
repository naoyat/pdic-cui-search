#ifndef DICT_H
#define DICT_H

#include <string>
#include <vector>
#include <map>

#include <cstdio>
#include "types.h"

class PDICIndex;

#define ENTRY_BUF_SIZE 1024*1024*128 // 128MBとりあえず

#define SUFFIX_ENTRY        ".entry"
#define SUFFIX_ENTRY_START  ".entry.st"
#define SUFFIX_ENTRY_SARRAY ".entry.sf"

class Dict {
 public:
  FILE *fp;
  std::string path, name;
  char *_suffix;

public:
  PDICIndex *index;
  byte *entry_buf;
  int *entry_start_pos, entry_start_pos_length;
  std::map<int,int> entry_start_rev;
  int *entry_suffix_array, entry_suffix_array_length;
  
public:
  Dict(FILE *fp, const std::string& name, const std::string& path);
  ~Dict();
  std::string info() { return name + " " + path; }
  char *suffix() { return _suffix; }

  int make_sarray_index(int buffer_size_enough_for_whole_entries =ENTRY_BUF_SIZE);
  bool load_additional_files();
  std::vector<int> search_in_sarray(byte *needle);
};

#endif;
