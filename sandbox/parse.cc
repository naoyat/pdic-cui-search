// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/parse.h"

#include <ctype.h>
#include <re2/re2.h>
#include <re2/stringpiece.h>

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "util/ansi_color.h"
#include "util/Shell.h"
#include "util/stlutil.h"
#include "util/types.h"
#include "util/util.h"
#include "sandbox/Word.h"
#include "sandbox/EnglishGrammar.h"

using namespace std;

map<int, pair<int, EnglishEntity*> > checked_ent;
map<int, pair<int, EnglishModifier*> > checked_mod;
map<int, pair<int, EnglishVerb*> > checked_verb;
map<int, pair<int, EnglishAdverb*> > checked_adverb;
map<int, pair<int, EnglishNP*> > checked_np;
map<int, pair<int, EnglishPP*> > checked_pp;
map<int, pair<int, EnglishVP*> > checked_vp;
map<int, pair<int, EnglishAP*> > checked_ap;
map<int, pair<int, EnglishSentence*> > checked_sentence;

EnglishEntity* parse_entity(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_ent.find(curr_ix) != checked_ent.end()) {
    *ix = checked_ent[curr_ix].first;
    return checked_ent[curr_ix].second;
  }

  Word* word = words[*ix];
  string surface = word->surface();
  string surface_ = strlower(surface);

  printf("  parse_ent(,%d)... ", *ix);
  cout << "surface=\"" << surface <<"\", pos=" << word->pos() << endl;

  EnglishEntity *ent = NULL;
  if (surface_ == "don't") {
    // do nothing
  } else if (isupper(surface[0])
             && (word->pos_canbe("人名") || word->pos_canbe("地名"))) {
    (*ix)++;
    ent = new EnglishName(word);
  } else if (word->pos_canbe("代名")) {
    (*ix)++;
    ent = new EnglishPronoun(word);
  } else if (surface_ != "in" && surface_ != "like"
             && (word->pos_canbe("名") || word->pos_canbe("名・形"))) {
    (*ix)++;
    ent = new EnglishNoun(word);
  }

  if (!ent) *ix = curr_ix;

  checked_ent[curr_ix] = make_pair(*ix, ent);
  return ent;
}

EnglishModifier* parse_modifier(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_mod.find(curr_ix) != checked_mod.end()) {
    *ix = checked_mod[curr_ix].first;
    return checked_mod[curr_ix].second;
  }

  Word* word = words[*ix];
  string surface_ = strlower(word->surface());

  printf("  parse_mod(,%d)... ", *ix);
  cout << "surface=\"" << surface_ <<"\", pos=" << word->pos() << endl;

  EnglishModifier* mod = NULL;
  if (surface_ == "the" || surface_ == "a" || surface_ == "an") {
    // a/an => "不" は良いが the => "副" なので
    (*ix)++;
    mod = new EnglishDeterminer(word);
  } else if (word->pos_canbe("形")) {
    (*ix)++;
    mod = new EnglishAdjective(word);
  }

  if (!mod) *ix = curr_ix;

  checked_mod[curr_ix] = make_pair(*ix, mod);
  return mod;
}

EnglishNP* parse_np(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_np.find(curr_ix) != checked_np.end()) {
    *ix = checked_np[curr_ix].first;
    return checked_np[curr_ix].second;
  }

  EnglishNP* np = NULL;
  vector<EnglishModifier*> modifiers;
  while (!np) {
    if (*ix == words.size()) break;

    Word* word = words[*ix];
    string surface_ = strlower(word->surface());

    printf("parse_np(,%d)... ", *ix);
    cout << "surface=\"" << surface_ <<"\", pos=" << word->pos() << endl;

    if (EnglishEntity* ent = parse_entity(words, ix)) {  // modよりentが先
      np = new EnglishNP(ent, modifiers);
    } else if (EnglishModifier* mod = parse_modifier(words, ix)) {
      modifiers.push_back(mod);
    } else {
      if (modifiers.size() > 0) {
        modifiers.clear();
      }
      break;
    }
  }

  if (!np) {
    *ix = curr_ix;
    checked_np[curr_ix] = make_pair(*ix, np);
    return np;
  }

  while (true) {
    if (*ix == words.size()) break;
    if (words[*ix]->pos_canbe("代名")) break;
    if (words[*ix]->pos_canbe("動")) break;
    if (words[*ix]->pos_canbe("自動")) break;
    if (words[*ix]->pos_canbe("他動")) break;

    EnglishEntity* ent = parse_entity(words, ix);
    if (!ent) break;

    printf("  +ent: %s\n", ent->surface().c_str());
    np->append_entity(ent);
  }

  while (true) {
    if (*ix == words.size()) break;

    EnglishPP* pp = parse_pp(words, ix);
    if (!pp) break;

    printf("  +pp: %s\n", pp->surface().c_str());
    np->add_pp(pp);
  }

  cout << ANSI_BOLD_ON << "[NP] " << np->surface() << ANSI_BOLD_OFF << endl;
  checked_np[curr_ix] = make_pair(*ix, np);
  return np;
}

EnglishPP* parse_pp(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_pp.find(curr_ix) != checked_pp.end()) {
    *ix = checked_pp[curr_ix].first;
    return checked_pp[curr_ix].second;
  }

  Word* word = words[*ix];
  string surface = word->surface();
  printf("parse_pp([%s,..], %d)...\n", surface.c_str(), *ix);

  EnglishPP* pp = NULL;
  if (word->pos_canbe("前")) {
    (*ix)++;
    EnglishPreposition* prep = new EnglishPreposition(word);

    /*
    if (strlower(surface) == "without") {
      EnglishSentence* s = parse_sentence(words, ix);
      if (s) {
      }
    }
    */
    EnglishNP* np = parse_np(words, ix);
    if (np) {
      pp = new EnglishPP(prep, np);
      cout << "[PP] " << pp->surface() << endl;
    } else {
      // back
    }
  }

  if (!pp) *ix = curr_ix;

  checked_pp[curr_ix] = make_pair(*ix, pp);
  return pp;
}

EnglishVerb* parse_verb(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_verb.find(curr_ix) != checked_verb.end()) {
    *ix = checked_verb[curr_ix].first;
    return checked_verb[curr_ix].second;
  }

  Word* word = words[*ix];
  string surface = word->surface();
  string surface_ = strlower(surface);
  printf("  parse_verb([%s,..], %d)...\n", surface.c_str(), *ix);

  EnglishVerb *verb = NULL;
  if (surface_ == "is" || surface_ == "are" || surface_ == "am"
      || surface_ == "was" || surface_ == "were"
      || surface_ == "isn't" || surface_ == "aren't"
      || surface_ == "wasn't" || surface_ == "weren't"
      || surface_ == "ain't"
      || surface_ == "won't" || surface == "wouldn't"
      || surface_ == "shouldn't" || surface_ == "couldn't") {
    (*ix)++;
    verb = new EnglishVerb(word);
  } else if (surface_ == "don't" || word->pos_canbe("助動")) {
    // don't は { 名, 助動 } なので扱いが厄介
    (*ix)++;
    verb = new EnglishVerb(word);
  } else if (word->pos_canbe("動")
             || word->pos_canbe("他動") || word->pos_canbe("自動")) {
    (*ix)++;
    verb = new EnglishVerb(word);
  }

  if (!verb) *ix = curr_ix;

  checked_verb[curr_ix] = make_pair(*ix, verb);
  return verb;
}

EnglishAdverb* parse_adverb(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_adverb.find(curr_ix) != checked_adverb.end()) {
    *ix = checked_adverb[curr_ix].first;
    return checked_adverb[curr_ix].second;
  }

  Word* word = words[*ix];
  string surface_ = strlower(word->surface());
  printf("  parse_adverb([%s,..], %d)...\n", word->surface().c_str(), *ix);

  EnglishAdverb *adverb = NULL;
  if (surface_ != "the" && word->pos_canbe("副")) {
    (*ix)++;
    adverb = new EnglishAdverb(word);
  }

  if (!adverb) *ix = curr_ix;

  checked_adverb[curr_ix] = make_pair(*ix, adverb);
  return adverb;
}

EnglishAP* parse_ap(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_ap.find(curr_ix) != checked_ap.end()) {
    *ix = checked_ap[curr_ix].first;
    return checked_ap[curr_ix].second;
  }

  Word* word = words[*ix];
  string surface = word->surface();
  printf("parse_ap([%s,..], %d)...\n", surface.c_str(), *ix);

  EnglishAdjective *adj = NULL;
  if (word->pos_canbe("形")) {
    (*ix)++;
    adj = new EnglishAdjective(word);
  }

  EnglishAP* ap = NULL;
  if (adj) {
    ap = new EnglishAP(adj);
    while (true) {
      if (*ix == words.size()) break;
      if (!words[*ix]->pos_canbe("形")) break;

      EnglishAdjective *adj = new EnglishAdjective(words[*ix]);
      (*ix)++;
      ap->append_adjective(adj);
    }
  }

  if (!adj) *ix = curr_ix;

  checked_ap[curr_ix] = make_pair(*ix, ap);
  return ap;
}

EnglishVP* parse_vp(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_vp.find(curr_ix) != checked_vp.end()) {
    *ix = checked_vp[curr_ix].first;
    return checked_vp[curr_ix].second;
  }

  Word* word = words[*ix];
  string surface_ = strlower(word->surface());
  printf("parse_vp([%s,..], %d)...\n", word->surface().c_str(), *ix);

  vector<EnglishAdverb*> adverbs;
  while (EnglishAdverb* adv = parse_adverb(words, ix)) {
    adverbs.push_back(adv);
  }

  EnglishVP *vp = NULL;

  EnglishVerb *verb = parse_verb(words, ix);
  if (!verb) {
    // *ix = curr_ix;
    checked_vp[curr_ix] = make_pair(*ix, vp);
    return NULL;
  }

  vp = new EnglishVP(verb);
  traverse(adverbs, it) {
    vp->add_adverb(*it);
  }

  while (true) {
    if (*ix == words.size()) break;
    if (words[*ix]->surface() == "like") break;

    EnglishVerb *verb = parse_verb(words, ix);
    if (!verb) break;
    vp->append_verb(verb);
  }

  if (vp) {
    while (true) {
      if (*ix == words.size()) break;

      if (EnglishAP* ap = parse_ap(words, ix)) {
        vp->add_ap(ap);
      } else if (EnglishNP* np = parse_np(words, ix)) {
        vp->add_np(np);
      } else if (EnglishPP* pp = parse_pp(words, ix)) {
        vp->add_pp(pp);
      } else if (EnglishAdverb* adv = parse_adverb(words, ix)) {
        vp->add_adverb(adv);
      } else {
        break;
      }
    }
    cout << ANSI_BOLD_ON << "[VP] " << vp->surface() << ANSI_BOLD_OFF << endl;
  }

  if (!vp) *ix = curr_ix;

  checked_vp[curr_ix] = make_pair(*ix, vp);
  return vp;
}

WObj* parse_conj(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;

  Word* word = words[*ix];
  string surface = words[*ix]->surface();
  string surface_ = strlower(surface);
  printf("  parse_conj([%s,..], %d)...\n", surface.c_str(), *ix);

  if ((*ix == 0 && surface_ == "that")
      || surface_ == "an") {
    *ix = curr_ix;
    return NULL;
  }

  if (word->pos_canbe("接続")) {
    (*ix)++;
    return new EnglishConjunction(word);
  }

  *ix = curr_ix;
  return NULL;
}

EnglishSentence* parse_sentence(const vector<Word*>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  if (checked_sentence.find(curr_ix) != checked_sentence.end()) {
    *ix = checked_sentence[curr_ix].first;
    return checked_sentence[curr_ix].second;
  }

  EnglishSentence* st = NULL;
  if (EnglishNP* np = parse_np(words, ix)) {
    if (*ix == words.size()) {
      // delete np;
      return NULL;
    }
    if (EnglishVP* vp = parse_vp(words, ix)) {
      st = new EnglishSentence(np, vp);
      // checked_sentence[curr_ix] = make_pair(*ix, st);
      // return st;
    } else {
      // delete np;
    }
  }

  if (!st) *ix = curr_ix;
  checked_sentence[curr_ix] = make_pair(*ix, st);
  return st;
}

vector<WObj*> parse(const vector<Word*>& words) {
  // printf("parse(, ix=%d)...\n", *ix);
  vector<WObj*> objs;

  checked_ent.clear();
  checked_mod.clear();
  checked_verb.clear();
  checked_adverb.clear();
  checked_np.clear();
  checked_pp.clear();
  checked_ap.clear();
  checked_vp.clear();
  checked_sentence.clear();

  for (unsigned int ix = 0, c = words.size(); ix < c; ) {
    printf("\n[LOOP] ix=%d ... ", ix);

    int curr_ix = ix;
    cout << ANSI_UNDERLINE_ON;
    for (unsigned int j = 0; j < ix; ++j) {
      cout << words[j]->surface() << " ";
    }
    cout << ANSI_FGCOLOR_RED;
    cout << ANSI_BOLD_ON << words[ix]->surface() << ANSI_BOLD_OFF;
    cout << ANSI_FGCOLOR_BLUE;
    for (unsigned int j = ix+1; j < c; ++j) {
      cout << " " << words[j]->surface();
    }
    cout << ANSI_FGCOLOR_DEFAULT << ANSI_UNDERLINE_OFF << endl;


    WObj *obj = NULL;
    while (!obj) {
      if (ix == words.size()) break;

      string surface = words[ix]->surface();
      string surface_ = strlower(surface);

      printf("\n[parse] ix=%d surface=\"%s\" ...\n", ix, surface.c_str());

      if (!obj) {
        // printf("  \"%s\", try conj...\n", s_);
        obj = parse_conj(words, &ix);
        if (obj) printf(" ! (%d-%d, [%s]) => conj. %s\n",
                        curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
      }
      if (!obj) {
        // printf("  \"%s\", try S...\n", s_);
        obj = parse_sentence(words, &ix);
        if (obj) printf(" ! (%d-%d, [%s]) => S %s\n",
                        curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
      }
      if (!obj) {
        // printf("  \"%s\", try NP...\n", s_);
        obj = parse_np(words, &ix);
        if (obj) printf(" ! (%d-%d, [%s]) => NP %s\n",
                        curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
      }
      if (!obj) {
        // printf("  \"%s\", try VP...\n", s_);
        obj = parse_vp(words, &ix);
        if (obj) printf(" ! (%d-%d, [%s]) => VP %s\n",
                        curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
      }
      if (!obj) {
        // printf("  \"%s\", try PP...\n", s_);
        obj = parse_pp(words, &ix);
        if (obj) printf(" ! (%d-%d, [%s]) => PP %s\n",
                        curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
      }
      if (!obj) {
        obj = words[ix++];
        if (obj) printf(" ! (%d-%d, [%s]) => ** %s\n",
                        curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
      }

      if (!obj) {
        if (obj) printf(" OBJ NOT FOUND! (%d-%d, [%s]) => ??\n",
                        curr_ix, ix-1, surface.c_str());
      }

      // if (obj) {
      // printf("  obj -> \"%s\", ix -> %d\n", obj->surface().c_str(), ix);
      // }
    }

    // cout << " // now pushing back (" << obj->surface() << ")" << endl;
    objs.push_back(obj);
  }

  return objs;
}
