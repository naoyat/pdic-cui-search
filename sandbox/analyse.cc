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
  int flags = LOOKUP_HENKAKEI;
  lookup_result_vec result_vec =
      g_shell->dicts[ej_dict_id]->henkakei_lookup(needle, flags);

  if (result_vec.size() == 0) {
    flags = LOOKUP_EXACT_MATCH | LOOKUP_HENKAKEI;
    result_vec =
        g_shell->dicts[ej_dict_id]->pdic_match_forward_lookup(needle, flags);
  }

  if (result_vec.size() == 0) return NULL;

  traverse(result_vec, it) {
    lookup_result result = *it;
    if (strcmp(reinterpret_cast<char*>(result[F_ENTRY]),
               reinterpret_cast<char*>(needle)) == 0)
      return result;
  }
  return result_vec[0];
}


void render_objs_as_table(vector<WObj*>& objs) {
  Einsatz ez(2);

  vector<pair<string, string> > styles;
  styles.push_back(make_pair(ANSI_BOLD_ON, ANSI_BOLD_OFF));
  styles.push_back(make_pair("", ""));
  ez.add_style_begins(styles);

  traverse(objs, it) {
    vector<string> vs;
    WObj* obj = (WObj*)(*it);
    // cout << (*it)->surface() << " " << (*it)->pos() << endl;
    // cout << " #surface# " << obj->surface() << endl;
    // cout << " #translate# " << obj->translate() << endl;
    vs.push_back(obj->surface());
    vs.push_back(obj->translate());
    // cout << " ## " << endl;
    traverse(obj->pos(), jt) {
      vs.push_back(*jt);
    }
    // cout << vs << endl;
    ez.add(vs);
  }
  ez.render();
}


void analyse_text(byte *text, int length) {
  re2::StringPiece input(reinterpret_cast<char*>(text), length);

  vector<string> tokens;
  while (true) {
    string token;
    if (!RE2::FindAndConsume(&input, "([^ ]+)", &token)) break;
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

  cout << tokens;
  if (last_ch)
    printf(", sentence type: (%c)", last_ch);  // ! ? .
  cout << endl;

  vector<Word> words;

  // 単語（スペースで区切られたトークン的な意味で）→ Wordクラスオブジェクト
  for (int i = 0; i < L; ++i) {
    /*
    int num;
    if (RE2::FullMatch(tokens[i], "(\\d+)", &num)) {
      // printf("(number %d)\n", num);
      words.push_back(new Word(NULL,
                               BYTE(const_cast<char*>(tokens[i].c_str()))));
      continue;
    }
    */
    string surface(tokens[i]);
    int surface_length = surface.size();

    switch (surface[surface_length-1]) {
      case ',': case ';': case ':':
        surface.resize(surface_length-1);
        break;
      default:
        break;
    }

    const int kVerb = 1, kNoun = 2;
    vector<pair<string,int> > candidates;

    // -s
    if (surface_length > 1) {
      string body   = surface.substr(0, surface_length-1);
      string suffix = surface.substr(surface_length-1, 1);
      if (suffix == "s")
        candidates.push_back(make_pair(body, kVerb|kNoun));
    }
    if (surface_length > 2) {
      string body   = surface.substr(0, surface_length-2);
      string suffix = surface.substr(surface_length-2, 2);
      if (suffix == "ed")
        candidates.push_back(make_pair(body, kVerb));
      if (suffix == "es")
        candidates.push_back(make_pair(body, kVerb|kNoun));
    }
    if (surface_length > 3) {
      string body   = surface.substr(0, surface_length-3);
      string suffix = surface.substr(surface_length-3, 3);
      if (suffix == "ing")
        candidates.push_back(make_pair(body, kVerb));
    }

    // lowercase
    if (i == 0) {
      std::string lower = strlower(surface);
      if (lower != surface) candidates.push_back(make_pair(lower, 0));
    }

    // as is
    candidates.push_back(make_pair(surface, 0));


    lookup_result result = NULL;

    vector<Word> possibilities;
    cout << surface << " =>";
    traverse(candidates, it) {
      string cand_surface = it->first;
      result = e_just(BYTE(const_cast<char*>(cand_surface.c_str())));
      if (result) {
        // Word word(result, cand_surface);
        Word word(result, surface);
        bool is_passed = false;
        int poss = it->second;
        if (poss == 0) {
          is_passed = true;
        }
        if (poss & kVerb) {
          if (word.pos_canbe("動")
              || word.pos_canbe("自動")
              || word.pos_canbe("他動"))
            is_passed = true;
        }
        if (poss & kNoun) {
          if (word.pos_canbe("名"))
            is_passed = true;
        }
        if (is_passed) {
          // cout << "\"" << it->surface() << "\", ";
          cout << surface << " (\"" << cand_surface << "\"), ";
          possibilities.push_back(word);
        }
      }
    }
    cout << endl;

    if (possibilities.size() > 0) {
      // Word word(result, surface);
      words.push_back(possibilities[0]);
      // cout << "==? ";
      /*
      if (words.size() > 0)
        words[0].dump(3);
      else
        cout << "words[] was empty..." << endl;
      */
    } else {
      printf("(%s) is not found..\n", tokens[i].c_str());
    }
  }

  // printf("before parsing......\n");
  //printf("words.size() = %d\n", (int)words.size());
  vector<WObj*> objs( parse(words) );
  // printf("after parsing......\n");


  // printf("tr<words>\n");
  /*
  traverse(words, it) {
    // cout << "type of object: " << (*it).type() << endl; // ok
    (*it).dump(0);
  }
  */

  // printf("MAYBE STILL ALIVE HERE...\n");

  render_objs_as_table(objs);
  /*

  //printf("objs.size() = %d\n", (int)objs.size());
  printf("tr<obj>\n");
  traverse(objs, it) {
    // printf("obj addr: %p %p\n",it, *it);
    // cout << " - " << (*it)->surface() << endl;
    // Word *w = (Word*)(*it);
    // w->dump(0);
    cout << "type of object: " << ((WObj*)*it)->type() << endl;
    ((WObj*)*it)->dump(0);
  }
  */
  
  /*
  cout << "word objects:";
  traverse(objs, it) {
    cout << " " << (*it)->surface();
  }
  cout << endl;
  */
}
