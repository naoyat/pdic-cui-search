// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/Einsatz.h"

#include <string>
#include <vector>

#include "util/stlutil.h"

Einsatz::Einsatz(int levels) {
  this->levels_ = levels;
  texts_.assign(levels, std::vector<std::string>());
  widths_.assign(levels, std::vector<int>());
  this->has_text_ = false;
}
Einsatz::~Einsatz() {
}

int width_of_text(const std::string& str) {
  int width = 0;
  for (unsigned int i = 0, c = str.size(); i < c; ) {
    if (0 <= str[i] && str[i] <= 0x7e) {
      width += 1;
      i += 1;
    } else {
      width += 2;
      i += 3;
    }
  }
  return width;
}

void Einsatz::add(std::vector<std::string> texts) {
  if (texts_.size() == 0) return;

  int curr_levels = texts.size();
  if (curr_levels > levels_) {
    int old_levels = levels_;
    texts_.resize(curr_levels);
    widths_.resize(curr_levels);
    levels_ = curr_levels;
    for (int y = old_levels; y < curr_levels; ++y) {
      for (unsigned int x = 0, c = texts_[0].size(); x < c; ++x) {
        texts_[y].push_back("");
        widths_[y].push_back(0);
      }
      style_begin_.push_back(style_begin_[old_levels-1]);
      style_end_.push_back(style_end_[old_levels-1]);
    }
  }

  int max_width = 0;
  for (int y = 0; y < levels_; ++y) {
    if (y < curr_levels) {
      int width = width_of_text(texts[y]);
      if (width > max_width) max_width = width;
      texts_[y].push_back(texts[y]);
      widths_[y].push_back(width);
    } else {
      texts_[y].push_back("");
      widths_[y].push_back(0);
    }
  }
  max_widths_.push_back(max_width);
}

void Einsatz::add_style_begins(std::vector<
                               std::pair<std::string, std::string> > styles) {
  for (int y = 0; y < levels_; ++y) {
    style_begin_.push_back(styles[y].first);
    style_end_.push_back(styles[y].second);
  }
  has_text_ = true;
}

void Einsatz::render() {
  for (int y = 0; y < levels_; ++y) {
    for (unsigned int x = 0, c = texts_[y].size(); x < c; ++x) {
      putchar('|');
      if (has_text_) printf("%s", style_begin_[y].c_str());
      printf("%s", texts_[y][x].c_str());
      for (int k = max_widths_[x] - widths_[y][x] + 1; k > 0; --k) putchar(' ');
      if (has_text_) printf("%s", style_end_[y].c_str());
    }
    putchar('|');
    putchar('\n');
  }
}
