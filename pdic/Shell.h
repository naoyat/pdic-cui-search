// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef UTIL_SHELL_H_
#define UTIL_SHELL_H_

#include <re2/re2.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "util/types.h"

class Dict;

class ShellParams {
 public:
  ShellParams();

  int set_render_count_limit(int limit);
  int set_lookup_mode(const char *mode_str);
  const char *get_lookup_mode();

 public:
  bool separator_mode;
  bool verbose_mode;
  bool direct_dump_mode;
  bool full_search_mode;
  bool ansi_coloring_mode;
  bool more_newline_mode;
  int  render_count_limit;
  bool stop_on_limit_mode;
  int  default_lookup_flags;
  int  debug_flags;
};


class Shell {
 public:
  Shell();
  ~Shell();

  void load_rc(const char *rcpath = NULL);

  int  do_load(const std::string& filename);
  bool do_command(char *cmdstr);
  void render_current_result();

  ShellParams params;

  std::vector<std::string> loadpaths;
  std::vector<Dict*> dicts;
  std::map<std::string, std::vector<std::string> > aliases;  // name -> name
  std::map<std::string, int> nametable;  // name -> dict_id
  std::string current_dict_name;

  std::vector<int> current_dict_ids;
  lookup_result_vec current_result_vec;
  RE2* current_pattern;
  std::pair<std::string, std::string> current_query;


 private:
  void do_alias(const std::string& alias,
                const std::string& valid_name);
  void do_alias(const std::string& alias,
                const std::vector<std::string>& valid_names);
  std::vector<int> resolve_aliases(const std::string& name);
  bool do_use(std::string name);

  void render_current_result(const std::set<int>& range);
  void render_status();
};

#endif  // UTIL_SHELL_H_
