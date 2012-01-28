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

class WObj {
 public:
  WObj();
  explicit WObj(byte *surface);
  explicit WObj(std::string surface);
  virtual ~WObj() {}

  virtual std::string surface() const { return surface_; }
  virtual std::vector<std::string> pos() const { return pos_; }
  virtual bool pos_canbe(const char *pos) const;

  virtual std::string translate() { return "---"; }
  virtual std::string translate(const std::string& pos) { return "---"; }
  virtual void dump(int indent = 0);

  static char *class_name(WObj *obj);

 protected:
  std::string surface_;
  std::vector<std::string> pos_;
};

class Word : public WObj {
 public:
  Word();
  explicit Word(lookup_result fields);
  Word(lookup_result fields, byte* surface);
  ~Word();

  void render();
  void render_full();

  virtual std::string translate();
  virtual std::string translate(const std::string& pos);
  // virtual void dump(int indent = 0);

 private:
  void parse_fields(lookup_result fields, byte* surface);

  lookup_result fields_;
  std::map<std::string, meanings_t> meanings_map_;
  std::vector<usage_t> usages_;
  std::map<std::string, std::string> info_;
};

#endif  // SANDBOX_WORD_H_
