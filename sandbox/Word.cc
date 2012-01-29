// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/Word.h"

#include <stdio.h>
#include <cxxabi.h>
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

//
// class WObj
//
WObj::WObj() : surface_(), pos_() {
  // printf("!!! ctor WOBj() called.\n");
  // this->surface_ = std::string("");
}

WObj::WObj(byte *surface) : surface_(reinterpret_cast<char*>(surface)), pos_() {
  // this->surface_ = std::string((char*)surface);
}

WObj::WObj(std::string surface) : surface_(surface), pos_() {
}

char *WObj::class_name(WObj *obj) {
  const std::type_info & id_p = typeid(*obj);
  int status;
  return abi::__cxa_demangle(id_p.name(), 0, 0, &status);
}

bool WObj::pos_canbe(const char *pos) const {
  const std::vector<std::string> my_pos = this->pos();
  for (unsigned int i = 0, c = my_pos.size(); i < c; ++i) {
    // printf("(canbe? %s vs %s)\b", pos, my_pos[i].c_str());
    if (pos == my_pos[i]) {
      // printf("(can be %s.)\n", pos);
      return true;
    }
  }
  // printf("(cannot be %s.)\n", pos);
  return false;
}

void WObj::dump(int indent) {
  cout << string(indent, ' ');
  cout << surface() << endl;
}

//
// class Word
//
Word::Word() : WObj(), meanings_map_(), usages_(), info_() {
  printf("!!! ctor Word() called.\n");
  // Word(NULL, (byte*)"");
  // this->fields_ = NULL;
  // this->surface_ = "";
}

Word::Word(lookup_result fields)
    : WObj(fields[F_ENTRY]), meanings_map_(), usages_(), info_() {
  printf("!!! ctor Word({\"%s\", ...}) called.\n",
         reinterpret_cast<char*>(fields[F_ENTRY]));
  parse_fields(fields, fields[F_ENTRY]);
}

Word::Word(lookup_result fields, byte *surface)
    : WObj(surface), meanings_map_(), usages_(), info_() {
  printf("!!! ctor Word({\"%s\", ...}, (byte*)surface=\"%s\") called.\n",
         reinterpret_cast<char*>(fields[F_ENTRY]),
         reinterpret_cast<char*>(surface));
  parse_fields(fields, surface);
}

Word::Word(lookup_result fields, std::string surface)
    : WObj(surface), meanings_map_(), usages_(), info_() {
  printf("!!! ctor Word({\"%s\", ...}, (std::string)surface=\"%s\") called.\n",
         reinterpret_cast<char*>(fields[F_ENTRY]), surface.c_str());
  parse_fields(fields,
               BYTE(const_cast<char*>(surface.c_str())));
}

void Word::parse_fields(lookup_result fields, byte *surface) {
  if (!fields) return;
  this->fields_ = (lookup_result)clone(fields, sizeof(fields[0])*F_COUNT);
  this->surface_ = std::string(reinterpret_cast<char*>(
                                   surface ? surface : fields_[F_ENTRY]));

  pair< map<string, meanings_t>, vector<usage_t> > result =
      parse_jword(fields_[F_JWORD]);

  // if (is_not_empty(fields[F_JWORD])) {
  meanings_map_ = result.first;
  usages_ = result.second;
  info_.clear();

  if (is_not_empty(fields_[F_PRON])) {
    info_["発音:IPA"] = reinterpret_cast<char*>(fields_[F_PRON]);
  }

  if (is_not_empty(fields[F_EXAMPLE])) {
    usages_.push_back(make_pair(make_pair("", 0),
                                make_pair(reinterpret_cast<char*>(
                                              fields_[F_EXAMPLE]), "")));
  }

  const char *to_moves[] = { "＠", "変化", "分節" };
  for (int i = 0; i < 3; ++i) {
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
          cout << "\t" << it->first << " : "
               << it->second.size() << "件" << endl;
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

std::string omit(string s) {
  vector<string> splitted = split(s, "、");
  string tr = splitted[0];
  RE2::GlobalReplace(&tr, "（.*）", "");
  RE2::GlobalReplace(&tr, "〔.*〕", "");
  RE2::GlobalReplace(&tr, "［.*］", "");
  RE2::GlobalReplace(&tr, "〈.*〉", "");
  RE2::GlobalReplace(&tr, "《.*》", "");

  //  RE2::GlobalReplace(&tr, "^\xef\xbd\x9eを", "");  // U+FF5E, "〜" in MS932
  RE2::GlobalReplace(&tr, "^\xef\xbd\x9e", "〜");  // U+FF5E, "〜" in MS932

  RE2::GlobalReplace(&tr, "…$", "");
  return tr;
}

std::string Word::translate() {
  traverse(pos_, it) {
    traverse(meanings_map_[*it], jt) {
      return omit(jt->second);
    }
  }
  return "*";
  // return "";  // string((char*)fields_[F_JWORD]);
}
/*
void Word::dump(int indent) {
  printf("Word::dump(%d)..\n", indent);
  // cout << string(indent, ' ');
  // cout << surface() << endl;
}
*/

std::string Word::translate(const std::string& pos) {
  if (meanings_map_.find(pos) == meanings_map_.end())
    return "";  // surface(); // std::string("---");

  meanings_t meanings = meanings_map_[pos];
  if (meanings.size() == 0) return surface();

  return omit(meanings[0].second);
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
