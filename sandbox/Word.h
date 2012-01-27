// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_WORD_H_
#define SANDBOX_WORD_H_

#include <re2/re2.h>

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "util/types.h"

// [(sub_id, meaning)]
typedef std::vector<std::pair<int, std::string> > meanings_t;
// [((pos, sub_id), (eng, jap))]
typedef std::pair<std::pair<std::string, int>,
                  std::pair<std::string, std::string> > usage_t;

class Word {
 public:
  Word();
  explicit Word(lookup_result fields, byte *surface = NULL);
  ~Word();

  std::string surface() const { return surface_; }
  std::vector<std::string> pos() const { return pos_; }

  void render();
  void render_full();

 private:
  std::string surface_;
  lookup_result fields_;
  std::map<std::string, meanings_t> meanings_map_;
  std::vector<usage_t> usages_;
  std::map<std::string, std::string> info_;
  std::vector<std::string> pos_;
};

#endif  // SANDBOX_WORD_H_
