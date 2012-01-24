// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/analyse.h"

#include <ctype.h>
#include <re2/re2.h>
#include <re2/stringpiece.h>

#include <iostream>
#include <string>
#include <vector>

#include "pdic/lookup.h"
#include "util/stlutil.h"
#include "util/types.h"

using namespace std;

void analyse_text(byte *text, int length) {
  re2::StringPiece input((char*)text, length);

  vector<string> tokens;

  string token;
  //int sentence_type = 0;
  while (RE2::FindAndConsume(&input, "([^ ]+)", &token)) {
    tokens.push_back(token);
  }

  int L = tokens.size();
  if (L == 0) return;

  if (tokens[0].size() >= 2) {
    if (!isupper(tokens[0][1]))
      tokens[0][0] = tolower(tokens[0][0]);
  }

  int last_ch = (*(tokens[L-1].end()-1));
  switch (last_ch) {
    case '?': case '!': case '.':
      tokens[L-1] = tokens[L-1].substr(0, tokens[L-1].size()-1);
      break;
    default:
      // last_ch = 
      break;
  }

  printf("sentence type: (%c)\n", last_ch);

  traverse(tokens, it) {
    string token = *it;
    lookup((byte*)token.c_str(), LOOKUP_EXACT_MATCH | LOOKUP_CASE_SENSITIVE);
  }
  // cout << tokens << "(" << (char)last_ch << ")" <<endl;
}
