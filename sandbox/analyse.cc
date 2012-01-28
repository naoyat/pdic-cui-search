// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/analyse.h"

#include <ctype.h>
#include <re2/re2.h>
#include <re2/stringpiece.h>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "pdic/Dict.h"
#include "pdic/lookup.h"
#include "util/ansi_color.h"
#include "util/Shell.h"
#include "util/stlutil.h"
#include "util/types.h"
#include "util/util.h"
#include "sandbox/alt.h"
#include "sandbox/Einsatz.h"
#include "sandbox/Word.h"
#include "sandbox/EnglishGrammar.h"
#include "sandbox/parse.h"

using namespace std;

extern Shell *g_shell;

extern bool alt_first_only;

//
// 英辞郎132のみを引き、needle にジャストマッチの単語を得る
//
lookup_result e_just(byte *needle) {
  std::string ej_dict("EIJI-132");
  // if not found
  if (g_shell->nametable.find(ej_dict) == g_shell->nametable.end())
    return (lookup_result)NULL;

  int ej_dict_id = g_shell->nametable[ej_dict];

  g_shell->params.direct_dump_mode = false;
  // int flags = LOOKUP_EXACT_MATCH | LOOKUP_CASE_SENSITIVE;
  int flags = LOOKUP_EXACT_MATCH;
  lookup_result_vec result_vec =
      g_shell->dicts[ej_dict_id]->pdic_match_forward_lookup(needle, flags);

  if (result_vec.size() == 0) return NULL;

  traverse(result_vec, it) {
    lookup_result result = *it;
    if (strcmp(reinterpret_cast<char*>(result[F_ENTRY]),
               reinterpret_cast<char*>(needle)) == 0)
      return result;
  }
  return result_vec[0];
}

void analyse_text(byte *text, int length) {
  re2::StringPiece input(reinterpret_cast<char*>(text), length);

  vector<string> tokens;

  string token;
  // int sentence_type = 0;
  while (RE2::FindAndConsume(&input, "([^ ]+)", &token)) {
    tokens.push_back(token);
  }

  int L = tokens.size();
  if (L == 0) return;

  int last_ch = (*(tokens[L-1].end()-1));
  switch (last_ch) {
    case '?': case '!': case '.':
      tokens[L-1] = tokens[L-1].substr(0, tokens[L-1].size()-1);
      break;
    default:
      last_ch = 0;
      break;
  }

  if (last_ch)
    printf("sentence type: (%c)\n", last_ch);  // ! ? .

  vector<Word*> words;

  // 単語（スペースで区切られたトークン的な意味で）→ Wordクラスオブジェクト
  for (int i = 0; i < L; ++i) {
    string surface(tokens[i]);
    int surface_length = surface.size();

    switch (surface[surface_length-1]) {
      case ',': case ';': case ':':
        surface.resize(surface_length-1);
        break;
      default:
        break;
    }

    vector<string> candidates;
    if (i == 0) {
      std::string lower = strlower(tokens[i]);
      if (lower != surface) candidates.push_back(lower);
    }
    candidates.push_back(surface);

    if (surface[surface_length - 1]  == 's') {
      candidates.push_back(surface.substr(0, surface_length-1));
    }

    lookup_result result = NULL;

    traverse(candidates, it) {
      result = e_just(BYTE(const_cast<char*>(it->c_str())));
      if (result) break;
    }

    if (!result) {
      token = tokens[i];
      result = e_just(BYTE(const_cast<char*>(token.c_str())));
    }

    words.push_back(new Word(result,
                             BYTE(const_cast<char*>(surface.c_str()))));
  }

  vector<WObj*> objs = parse(words);


  traverse(objs, it) {
    (*it)->dump(0);
    // cout << " - " << (*it)->surface() << endl;
  }
  /*
  cout << "word objects:";
  traverse(objs, it) {
    cout << " " << (*it)->surface();
  }
  cout << endl;
  */
  Einsatz ez(2);

  vector<pair<string, string> > styles;
  styles.push_back(make_pair(ANSI_BOLD_ON, ANSI_BOLD_OFF));
  styles.push_back(make_pair("", ""));
  ez.add_style_begins(styles);

  traverse(objs, it) {
    vector<string> vs;
    // cout << (*it)->surface() << " " << (*it)->pos() << endl;
    vs.push_back((*it)->surface());
    vs.push_back((*it)->translate());
    vector<string> poss = (*it)->pos();
    traverse(poss, jt) {
      vs.push_back(*jt);
    }
    // cout << vs << endl;
    ez.add(vs);
  }
  ez.render();

  /*
  if (last_ch) cout << (char)last_ch;
  cout << ANSI_BOLD_OFF << endl;
  */

  /*
  traverse(words, it) {
    (*it)->render();
  }
  */

  // delete all
  traverse(words, it) {
    delete *it;
  }
}
