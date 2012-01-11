#ifndef DICT_H
#define DICT_H

#include <string>
#include <cstdio>

class PDICIndex;

class Dict {
 public:
  FILE *fp;
  PDICIndex *index;
  std::string path, name;
  char *_suffix;

public:
  Dict(FILE *fp, const std::string& name, const std::string& path);
  ~Dict();
  std::string info() { return name + " " + path; }
  char *suffix() { return _suffix; }

  int make_sarray_index();
  int load_sarray_index();
};

#endif;
