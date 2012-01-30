// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/Wobj.h"

#include <stdio.h>
#include <re2/re2.h>

#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

using namespace std;

#include "pdic/Dict.h"
#include "util/ansi_color.h"
#include "util/stlutil.h"
#include "util/types.h"
#include "util/util.h"
#include "sandbox/parse_lookup_result.h"

WObj::WObj() : surface_(), pos_() {
  // printf("WObj();\n");
  this->surface_ = "";
  this->the_pos_ = NULL;
  this->pos_.clear();
}

WObj::WObj(byte *surface) : surface_(reinterpret_cast<char*>(surface)), pos_() {
  printf("WObj(surface:\"%s\");\n", (char*)surface);
  // this->surface_ = std::string((char*)surface);
  // printf("WObj: setting surface {%s}.\n", (char*)surface);
  this->surface_ = std::string(reinterpret_cast<char*>(surface));
  this->the_pos_ = NULL;
  this->pos_.clear();
}

WObj::WObj(std::string& surface) : surface_(surface), pos_() {
  printf("WObj((std::string)surface:\"%s\");\n", surface.c_str());
  // printf("WObj: setting surface {%s}.\n", surface.c_str());
  this->surface_ = surface;
  this->the_pos_ = NULL;
  this->pos_.clear();
}

WObj::WObj(const WObj& wobj) {
  printf("WObj(copy:wobj \"%s\");\n", wobj.surface().c_str());
  this->surface_ = wobj.surface_;
  this->the_pos_ = wobj.the_pos_;
  this->pos_.assign(wobj.pos_.begin(), wobj.pos_.end());
}

WObj& WObj::operator=(const WObj& wobj) {
  printf("WObj op=(wobj \"%s\");\n", wobj.surface().c_str());
  this->surface_ = wobj.surface_;
  this->the_pos_ = wobj.the_pos_;
  this->pos_.assign(wobj.pos_.begin(), wobj.pos_.end());
  return *this;
}

WObj::~WObj() {
  // printf("~WObj();\n");
}

bool WObj::pos_canbe(const char *pos) {
  const std::vector<const char*> my_pos = this->pos_;
  traverse(this->pos_, it) {
    if (strcmp(pos, *it) == 0) return true;
  }
  /*
  for (unsigned int i = 0, c = my_pos.size(); i < c; ++i) {
    // printf("(canbe? %s vs %s)\b", pos, my_pos[i].c_str());
    if (pos == my_pos[i]) {
      // printf("(can be %s.)\n", pos);
      return true;
    }
  }
  */
  // printf("(cannot be %s.)\n", pos);
  return false;
}

std::string WObj::translate() {
  if (the_pos_)
    return translate_with_pos(the_pos_);
  else
    traverse(pos_, it) return translate_with_pos(*it);

  return "*";
}

std::string WObj::translate_with_pos(const char *pos) {
  return "**";
}

void WObj::dump(int indent) {
  cout << string(indent, ' ');
  cout << surface() << endl;
}
