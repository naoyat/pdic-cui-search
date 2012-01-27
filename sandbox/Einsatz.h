// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_EINSATZ_H_
#define SANDBOX_EINSATZ_H_

#include <string>
#include <utility>
#include <vector>

class Einsatz {
 public:
  Einsatz(int levels);
  ~Einsatz();

  void add(std::vector<std::string> texts);
  void add_style_begins(std::vector<
                        std::pair<std::string, std::string> > styles);
  void render();

 private:
  bool has_text_;
  int levels_;
  std::vector<std::vector<std::string> > texts_;
  std::vector<std::vector<int> > widths_;
  std::vector<int> max_widths_;
  std::vector<std::string> style_begin_, style_end_;
};

#endif  // SANDBOX_EINSATZ_H_
