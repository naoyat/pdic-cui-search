// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/Word.h"

#include <stdio.h>
#include <re2/re2.h>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "pdic/Dict.h"
#include "util/ansi_color.h"
#include "util/stlutil.h"
#include "util/types.h"
#include "util/util.h"
#include "sandbox/parse_lookup_result.h"

//
// class Word
//
Word::Word() :
    surface_(), meanings_map_(), usages_(), info_(), pos_() {
  this->fields_ = NULL;
}

Word::Word(lookup_result fields, byte *surface) :
    surface_(), meanings_map_(), usages_(), info_(), pos_() {
  if (!fields) return;

  this->fields_ = (lookup_result)clone(fields, sizeof(fields[0])*F_COUNT);
  this->surface_ = std::string((char*)(
                                     surface ? surface : fields_[F_ENTRY]));

  pair< map<string, meanings_t>, vector<usage_t> > result =
      parse_jword(fields_[F_JWORD]);

  // if (is_not_empty(fields[F_JWORD])) {
  meanings_map_ = result.first;
  usages_ = result.second;
  info_.clear();

  if (is_not_empty(fields_[F_PRON])) {
    info_["発音:IPA"] = (char*)fields_[F_PRON];
  }

  if (is_not_empty(fields[F_EXAMPLE])) {
    usages_.push_back(make_pair(make_pair("", 0),
                                make_pair((char*)fields_[F_EXAMPLE], "")));
  }

  const char *to_moves[] = { "＠", "変化", "分節" };
  for (int i=0; i<3; ++i) {
    string what_to_move(to_moves[i]);
    if (meanings_map_.find(what_to_move) != meanings_map_.end()) {
      info_[what_to_move] = meanings_map_[what_to_move][0].second;
      meanings_map_.erase(what_to_move);
    }
  }

  pos_.clear();
  if (meanings_map_.size() > 0) {
    traverse(meanings_map_, it) {
      pos_.push_back((*it).first);
    }
  }
}

Word::~Word() {
  // if (fields_) free((void*)fields_);
}

//
// render
//
void Word::render_full() {
  printf("{\n");
  if (is_not_empty(fields_[F_ENTRY])) {
    printf("  見出し語: " ANSI_FGCOLOR_RED "%s" ANSI_FGCOLOR_DEFAULT "\n",
           fields_[F_ENTRY]);
  }

  if (is_not_empty(fields_[F_JWORD])) {
    if (meanings_map_.size() > 0) {
      cout << "  意味: " << endl;
      traverse(meanings_map_, it) {
        if (it->first == "*") {
          traverse(it->second, jt) {
            cout << "\t(" << jt->first << ") " << jt->second << endl;
          }
        } else {
          cout << "\t" << it->first << " : " << it->second.size() << "件" << endl;
          traverse(it->second, jt) {
            cout << "\t\t(" << jt->first << ") " << jt->second << endl;
          }
        }
      }
    }

    if (info_.size() > 0) {
      cout << "  付加情報: " << endl;
      traverse(info_, it) {
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

void Word::render() {
  if (!fields_) {
    printf("{\n  not found: %s\n}\n", surface_.c_str());
    return;
  }

  cout << ANSI_FGCOLOR_RED << fields_[F_ENTRY];
  if (is_not_empty(fields_[F_PRON])) {
    cout << ANSI_FGCOLOR_BLUE << " [" << fields_[F_PRON] << "]";
  }
  cout << ANSI_FGCOLOR_DEFAULT << " ";

  // POS
  traverse(pos_, it) cout << *it << " ";

  cout << "{" << endl;

  if (meanings_map_.size() > 0) {
    traverse(meanings_map_, it) {
      cout << "    " << it->first << " : ";
      traverse(it->second, jt) {
        cout << "(" << jt->first << ") " << jt->second << " ";
      }
      cout << endl;
    }
  }
  /*
  if (info.size() > 0) {
    // cout << "  付加情報: " << endl;
    if (info.find("変化") != info.end()) {
      cout << "    変化 : " << info["変化"] << endl;
    }
  }
  */
  printf("}\n");
}
