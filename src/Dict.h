#ifndef DICT_H
#define DICT_H

#include <string>
#include <vector>
#include <map>

#include <cstdio>
#include "types.h"

#include <re2/re2.h>

class PDICIndex;

#define ENTRY_BUF_SIZE 1024*1024*128 // 128MBとりあえず

#define SUFFIX_ENTRY        ".entry"
#define SUFFIX_ENTRY_START  ".entry.st"
#define SUFFIX_ENTRY_SARRAY ".entry.sf"

//typedef std::pair<std::string,std::string> lookup_result;
typedef std::pair<byte*,byte*> lookup_result;
typedef std::vector<lookup_result> lookup_result_vec;

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
  void unload_additional_files();
  bool load_additional_files();

private:
  std::vector<int> search_in_sarray(byte *needle);
public:
  lookup_result_vec normal_lookup(byte *needle, bool exact_match);
  lookup_result_vec sarray_lookup(byte *needle);
  lookup_result_vec regexp_lookup(const RE2& pattern);
};


// render
void render_ej(lookup_result result);

// CALLBACKS
void dump_ej(PDICDatafield *datafield);
void dump_entry(PDICDatafield *datafield);
void dump_to_vector(PDICDatafield *datafield);

#endif;
