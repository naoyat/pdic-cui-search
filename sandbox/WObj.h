// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_WOBJ_H_
#define SANDBOX_WOBJ_H_

#include <re2/re2.h>

#include <string>
#include <vector>

#include "util/types.h"

class WObj {
 public:
  WObj();
  explicit WObj(byte *surface);
  explicit WObj(std::string surface);
  virtual ~WObj();

  WObj(const WObj& wobj);
  WObj& operator=(const WObj& wobj);

  virtual const char* type() { return "WObj"; }

  virtual std::string surface() const { return surface_; }
  virtual std::vector<std::string> pos() const { return pos_; }
  virtual bool pos_canbe(const char *pos) const;

  virtual std::string translate();
  virtual std::string translate_with_pos(const char *pos);
  virtual void dump(int indent = 0);

  // static char *class_name(WObj *obj);

  void set_the_pos(const char *the_pos) {
    the_pos_ = the_pos;
  }

 protected:
  std::string surface_;
  std::vector<std::string> pos_;

  const char *the_pos_;  // 品詞が確定している場合にセット
};

#endif  // SANDBOX_WOBJ_H_
