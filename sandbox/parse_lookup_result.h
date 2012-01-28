// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_PARSE_LOOKUP_RESULT_H_
#define SANDBOX_PARSE_LOOKUP_RESULT_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "util/types.h"
#include "sandbox/Word.h"

std::pair<std::string, std::string> split_line(char *begin, char *end);
std::pair<std::string, std::string> parse_usage(char **begin, char *end);
std::pair<std::string, int> parse_pos(char **begin, char *end);
std::vector<std::string> parse_line(char **begin, char *end);
std::pair<std::map<std::string, meanings_t>, std::vector<usage_t> >
    parse_jword(byte* jword);

#endif  // SANDBOX_PARSE_LOOKUP_RESULT_H_
