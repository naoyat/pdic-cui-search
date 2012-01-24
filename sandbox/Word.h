// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_WORD_H_
#define SANDBOX_WORD_H_

#include <re2/re2.h>

#include <string>
#include <utility>
#include <vector>

#include "util/types.h"

// [(sub_id, meaning)]
typedef std::vector<std::pair<int, std::string> > meanings_t;
// [((pos, sub_id), (eng, jap))]
typedef std::pair<std::pair<std::string, int>,
                  std::pair<std::string, std::string> > usage_t;

std::pair<std::string, std::string> split_line(char *begin, char *end);
std::pair<std::string, std::string> parse_usage(char **begin, char *end);
std::pair<std::string, int> parse_pos(char **begin, char *end);
std::vector<std::string> parse_line(char **begin, char *end);
std::pair<std::map<std::string, meanings_t>, std::vector<usage_t> > parse_jword(byte* jword);


class Word {
public:
  Word();
  explicit Word(lookup_result fields, byte *surface = NULL);
  ~Word();

  std::string surface() { return surface_; }

  void render();
  void render_full();


private:
  std::string surface_;
  lookup_result fields_;
  std::map<std::string, meanings_t> meanings_map_;
  std::vector<usage_t> usages_;
  std::map<std::string, std::string> info_;

};

#endif  // SANDBOX_WORD_H_
