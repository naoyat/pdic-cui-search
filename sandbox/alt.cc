// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/alt.h"

#include <stdio.h>
#include <string.h>
#include <re2/re2.h>
#include <re2/stringpiece.h>

#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "pdic/Dict.h"
#include "util/ansi_color.h"
#include "util/Shell.h"
#include "util/stlutil.h"
#include "util/types.h"
#include "util/util.h"
#include "sandbox/Word.h"

extern Shell *shell;

bool alt_show_usage = false;
bool alt_show_remark = false;

bool alt_first_only = false;

using namespace std;

//
// alternative renderers
//
void render_result_alt(lookup_result fields, RE2 *re) {
  if (alt_first_only) {
    render_result_alt1(fields, re);
  } else {
    render_result_altf(fields, re);
  }
}

void render_result_alt1(lookup_result fields, RE2 *re) {
  Word word(fields);
  word.render();
}

void render_result_altf(lookup_result fields, RE2 *re) {
  printf("{\n");
  if (is_not_empty(fields[F_ENTRY])) {
    printf("  見出し語: " ANSI_FGCOLOR_RED "%s" ANSI_FGCOLOR_DEFAULT "\n",
           fields[F_ENTRY]);
  }

  if (is_not_empty(fields[F_JWORD])) {
    pair< map<string, meanings_t>, vector<usage_t> > result = parse_jword(fields[F_JWORD]);

    map<string, meanings_t> meanings_map = result.first;
    vector<usage_t> usages = result.second;
    map<string, string> info;

    if (is_not_empty(fields[F_PRON])) {
      info["発音:IPA"] = (char*)fields[F_PRON];
    }

    const char *to_moves[] = { "＠", "変化", "分節" };
    for (int i=0; i<3; ++i) {
      string what_to_move(to_moves[i]);
      if (meanings_map.find(what_to_move) != meanings_map.end()) {
        info[what_to_move] = meanings_map[what_to_move][0].second;
        meanings_map.erase(what_to_move);
      }
    }

    if (is_not_empty(fields[F_EXAMPLE])) {
      usages.push_back(make_pair(make_pair("", 0), make_pair((char*)fields[F_EXAMPLE], "")));
    }

    if (meanings_map.size() > 0) {
      cout << "  意味: " << endl;
      traverse(meanings_map, it) {
        if (it->first == "*") {
          traverse(it->second, jt) {
            cout << "\t(" << jt->first << ") " << jt->second << endl;
            if (alt_first_only) break;
          }
        } else {
          cout << "\t" << it->first << " : " << it->second.size() << "件" << endl;
          traverse(it->second, jt) {
            cout << "\t\t(" << jt->first << ") " << jt->second << endl;
            if (alt_first_only) break;
          }
        }
      }
    }

    if (info.size() > 0) {
      cout << "  付加情報: " << endl;
      traverse(info, it) {
        cout << "\t" << it->first << " : " << it->second << endl;
      }
    }
#if 0
    if (usages.size() > 0) {
      cout << "  用例: " << endl;
      traverse(usages, it) {
        // cout << *it << endl;
        cout << "\t" << it->first << endl;
        cout << "\t\t" << it->second.first << endl;
        cout << "\t\t" << it->second.second << endl;
      }
    }
#endif
  }
  printf("}\n");
}
