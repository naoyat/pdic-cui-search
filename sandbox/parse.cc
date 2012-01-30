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

#define CHK_ENTITY       1
#define CHK_MODIFIER     2
#define CHK_VERB         3
#define CHK_BE           4
#define CHK_ADVERB       5
#define CHK_CONJUNCTION  6
#define CHK_NP          10
#define CHK_VP          11
#define CHK_PP          12
#define CHK_AP          13
#define CHK_SENTENCE    20

map<unsigned int, pair<int, void*> > chk_memo;

void clear_memo() {
  traverse(chk_memo, it) {
    WObj* obj = (WObj*)(*it).second.second;
    delete obj;
  }
  chk_memo.clear();
}

void* get_memo(unsigned int chk_type, unsigned int* ix) {
  unsigned int key = (chk_type << 20) | *ix;
  if (chk_memo.find(key) != chk_memo.end()) {
    printf("get_memo: we have memoized one! (type=%d) ix: %d->%d, %p\n",
           chk_type, *ix, chk_memo[key].first, chk_memo[key].second);
    *ix = chk_memo[key].first;
    return chk_memo[key].second;
  }
  return NULL;
}

void *set_memo(unsigned int chk_type,
              unsigned int begin_ix, unsigned int end_ix,
              void *obj) {
  if (end_ix <= begin_ix && obj != NULL) {
    printf("set_memo: invalid ix! (type=%d): %d->%d\n", chk_type, begin_ix, end_ix);
  }
  unsigned int key = (chk_type << 20) | begin_ix;
  chk_memo[key] = make_pair(end_ix, obj);
  return obj;  // thru
}


EnglishEntity* parse_entity(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishEntity* ent = static_cast<EnglishEntity*>(get_memo(CHK_ENTITY, ix));
  if (ent) return ent;

  Word* word = &words[*ix];
  string surface = word->surface();
  string surface_ = strlower(surface);

  printf("   - parse_ent(,%d)... ", *ix);
  cout << "surface=\"" << surface <<"\", pos=" << word->pos() << endl;

  if (surface == "") {
    // do nothing (but why empty)
  } else if (surface_ == "don't") {
    // do nothing
  } else if (isupper(surface[0])
             && (word->pos_canbe("人名") || word->pos_canbe("地名"))) {
    (*ix)++;
    ent = new EnglishName(*word);
  } else if (surface == "I" || surface_ == "we"
             || surface_ == "you"
             || surface_ == "he" || surface_ == "she" || surface_ == "it"
             || surface == "they") {
    (*ix)++;
    ent = new EnglishPronoun(*word);
  } else if (word->pos_canbe("代名")) {
    (*ix)++;
    ent = new EnglishPronoun(*word);
  } else if (surface_ != "in" && surface_ != "like"
             && (word->pos_canbe("名") || word->pos_canbe("名・形"))) {
    (*ix)++;
    ent = new EnglishNoun(*word);
  }

  return static_cast<EnglishEntity*>(
      ent
      ? set_memo(CHK_ENTITY, curr_ix, *ix, ent)
      : set_memo(CHK_ENTITY, curr_ix, *ix=curr_ix, NULL));
}  // parse_entity


EnglishModifier* parse_modifier(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishModifier* mod =
      static_cast<EnglishModifier*>(get_memo(CHK_MODIFIER, ix));
  if (mod) return mod;

  Word* word = &words[*ix];
  string surface_ = strlower(word->surface());

  printf("   - parse_mod(,%d)... ", *ix);
  cout << "surface=\"" << surface_ <<"\", pos=" << word->pos() << endl;

  if (surface_ == "the" || surface_ == "a" || surface_ == "an") {
    // a/an => "不" は良いが the => "副" なので
    (*ix)++;
    mod = new EnglishDeterminer(*word);
  } else if (word->pos_canbe("形")) {
    (*ix)++;
    mod = new EnglishAdjective(*word);
  }

  return static_cast<EnglishModifier*>(
      mod
      ? set_memo(CHK_MODIFIER, curr_ix, *ix, mod)
      : set_memo(CHK_MODIFIER, curr_ix, *ix=curr_ix, NULL));
}  // parse_modifier


EnglishNP* parse_np(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishNP* np = static_cast<EnglishNP*>(get_memo(CHK_NP, ix));
  if (np) return np;

  vector<EnglishModifier*> modifiers;
  printf("NP 01\n");
  while (!np) {
    printf("NP 02 (%d/%d)\n", *ix, (int)words.size());
    if (*ix == words.size()) break;

    printf("NP 03\n");
    Word* word = &words[*ix];
    string surface_ = strlower(word->surface());

    printf(" - parse_np(,%d)... ", *ix);
    cout << "surface=\"" << surface_ <<"\", pos=" << word->pos() << endl;

    if (EnglishEntity* ent = parse_entity(words, ix)) {  // modよりentが先
      np = new EnglishNP(ent, modifiers);
    } else if (EnglishModifier* mod = parse_modifier(words, ix)) {
      modifiers.push_back(mod);
    } else {
      if (modifiers.size() > 0) {
        // traverse(modifiers, it) delete *it;
        modifiers.clear();
      }
      break;
    }
  }

  printf("NP 10\n");
  if (!np) {
    return static_cast<EnglishNP*>(
        set_memo(CHK_NP, curr_ix, *ix=curr_ix, NULL));
  }

  printf("NP 11\n");
  while (true) {
    printf("NP 12 (%d/%d)\n", *ix, (int)words.size());
    if (*ix == words.size()) break;
    if (words[*ix].pos_canbe("代名")) break;
    if (words[*ix].pos_canbe("動")) break;
    if (words[*ix].pos_canbe("自動")) break;
    if (words[*ix].pos_canbe("他動")) break;

    EnglishEntity* ent = parse_entity(words, ix);
    if (!ent) break;

    printf("  +ent: %s\n", ent->surface().c_str());
    np->append_entity(ent);
  }

  printf("NP 21\n");
  while (true) {
    printf("NP 22 (%d/%d)\n", *ix, (int)words.size());
    if (*ix == words.size()) break;

    EnglishPP* pp = parse_pp(words, ix);
    if (!pp) break;

    printf("  +pp: %s\n", pp->surface().c_str());
    np->add_pp(pp);
  }

  printf("NP 31\n");
  // np != NULL
  cout << ANSI_BOLD_ON << "[NP] " << np->surface() << ANSI_BOLD_OFF << endl;
  np->dump();
  return static_cast<EnglishNP*>(set_memo(CHK_NP, curr_ix, *ix, np));
}  // parse_np


EnglishPP* parse_pp(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishPP* pp = static_cast<EnglishPP*>(get_memo(CHK_PP, ix));
  if (pp) return pp;

  Word* word = &words[*ix];
  string surface = word->surface();
  printf(" - parse_pp([%s,..], %d)...\n", surface.c_str(), *ix);

  if (word->pos_canbe("前")) {
    EnglishPreposition* prep = new EnglishPreposition(words[(*ix)++]);

    EnglishNP* np = parse_np(words, ix);
    if (np) {
      pp = new EnglishPP(prep, np);
      cout << "[PP] " << pp->surface() << endl;
    } else {
      // back
    }
  }

  return static_cast<EnglishPP*>(
      pp
      ? set_memo(CHK_PP, curr_ix, *ix, pp)
      : set_memo(CHK_PP, curr_ix, *ix=curr_ix, NULL));
}  // parse_pp


bool is_have_verb_surface(string surface) {
  string surface_ = strlower(surface);
  if (surface_ == "have" || surface_ == "haven't"
      || surface_ == "has" || surface_ == "hasn't"
      || surface_ == "had" || surface_ == "hadn't") {
    return true;
  } else {
    return false;
  }
}

bool is_be_verb_surface(string surface) {
  string surface_ = strlower(surface);
  if (surface_ == "be" || surface_ == "been"
      || surface_ == "is" || surface_ == "are" || surface_ == "am"
      || surface_ == "was" || surface_ == "were"
      || surface_ == "isn't" || surface_ == "aren't"
      || surface_ == "wasn't" || surface_ == "weren't") {
    return true;
  } else {
    return false;
  }
}


EnglishBe* parse_be(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishBe* be = static_cast<EnglishBe*>(get_memo(CHK_BE, ix));
  if (be) return be;

  Word* word = &words[*ix];
  string surface = word->surface();
  string surface_ = strlower(surface);
  printf("   - parse_be([%s,..], %d)...\n", surface.c_str(), *ix);

  if (is_have_verb_surface(surface)
      && (*ix)+1 < words.size()
      && is_be_verb_surface(words[(*ix)+1].surface())) {
    // be = new EnglishBe(words[*ix]);
    (*ix)++;
    be = new EnglishBe(words[*ix]);
    // be->append_verb(words[*ix]);
    (*ix)++;
  } else if (is_be_verb_surface(surface)) {
    be = new EnglishBe(words[*ix]);
    (*ix)++;
  }

  return static_cast<EnglishBe*>(
      be
      ? set_memo(CHK_BE, curr_ix, *ix, be)
      : set_memo(CHK_BE, curr_ix, *ix=curr_ix, NULL));
}  // parse_be


EnglishVerb* parse_verb(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishVerb* verb = static_cast<EnglishVerb*>(get_memo(CHK_VERB, ix));
  if (verb) return verb;

  Word* word = &words[*ix];
  string surface = word->surface();
  string surface_ = strlower(surface);
  printf("   - parse_verb([%s,..], %d)...\n", surface.c_str(), *ix);

  if (surface_ == "won't" || surface == "wouldn't"
      || surface_ == "shouldn't" || surface_ == "couldn't") {
    (*ix)++;
    verb = new EnglishVerb(*word);
  } else if (surface_ == "don't" || word->pos_canbe("助動")) {
    // don't は { 名, 助動 } なので扱いが厄介
    (*ix)++;
    verb = new EnglishVerb(*word);
  } else if (word->pos_canbe("動")
             || word->pos_canbe("他動") || word->pos_canbe("自動")) {
    (*ix)++;
    verb = new EnglishVerb(*word);
  }

  return static_cast<EnglishVerb*>(
      verb
      ? set_memo(CHK_VERB, curr_ix, *ix, verb)
      : set_memo(CHK_VERB, curr_ix, *ix=curr_ix, NULL));
}  // parse_verb


EnglishAdverb* parse_adverb(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishAdverb* adv = static_cast<EnglishAdverb*>(get_memo(CHK_ADVERB, ix));
  if (adv) return adv;

  Word* word = &words[*ix];
  string surface_ = strlower(word->surface());
  printf("   - parse_adverb([%s,..], %d)...\n", word->surface().c_str(), *ix);

  if (surface_ != "the" && word->pos_canbe("副")) {
    (*ix)++;
    adv = new EnglishAdverb(*word);
  }

  return static_cast<EnglishAdverb*>(
      adv
      ? set_memo(CHK_ADVERB, curr_ix, *ix, adv)
      : set_memo(CHK_ADVERB, curr_ix, *ix=curr_ix, NULL));
}  // parse_adverb


EnglishAP* parse_ap(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishAP* ap = static_cast<EnglishAP*>(get_memo(CHK_AP, ix));
  if (ap) return ap;

  Word* word = &words[*ix];
  string surface = word->surface();
  printf(" - parse_ap([%s,..], %d)...\n", surface.c_str(), *ix);

  EnglishAdjective *adj = NULL;
  if (word->pos_canbe("形")) {
    (*ix)++;
    adj = new EnglishAdjective(*word);
  }

  if (adj) {
    ap = new EnglishAP(adj);
    while (true) {
      if (*ix == words.size()) break;
      if (!words[*ix].pos_canbe("形")) break;

      EnglishAdjective *adj = new EnglishAdjective(words[*ix]);
      (*ix)++;
      ap->append_adjective(adj);
    }
  }

  return static_cast<EnglishAP*>(
      ap
      ? set_memo(CHK_AP, curr_ix, *ix, ap)
      : set_memo(CHK_AP, curr_ix, *ix=curr_ix, NULL));
}  // parse_ap


EnglishVP* parse_vp(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishVP* vp = static_cast<EnglishVP*>(get_memo(CHK_VP, ix));
  if (vp) return vp;

  Word* word = &words[*ix];
  string surface_ = strlower(word->surface());
  printf(" - parse_vp([%s,..], %d)...\n", word->surface().c_str(), *ix);

  vector<EnglishAdverb*> adverbs;
  while (EnglishAdverb* adv = parse_adverb(words, ix)) {
    adverbs.push_back(adv);
  }

  if (EnglishBe* be = parse_be(words, ix)) {
    vp = new EnglishVP(be);
    //    traverse(adverbs, it) {
    //      vp->add_adverb(*it);
    //    }
    if (EnglishAP* ap = parse_ap(words, ix)) {
      vp->add_ap(ap);
    }
  } else {
    EnglishVerb *verb = parse_verb(words, ix);
    if (!verb) {
      return static_cast<EnglishVP*>(
          set_memo(CHK_VP, curr_ix, *ix=curr_ix, NULL));
    }

    vp = new EnglishVP(verb);
    traverse(adverbs, it) {
      vp->add_adverb(*it);
    }
  }

  while (true) {
    if (*ix == words.size()) break;
    if (words[*ix].surface() == "like") break;

    EnglishVerb *verb = parse_verb(words, ix);
    if (!verb) break;
    vp->append_verb(verb);
  }

  if (vp) {
    while (true) {
      if (*ix == words.size()) break;

      if (EnglishNP* np = parse_np(words, ix)) {
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
    vp->dump();
  }

  return static_cast<EnglishVP*>(
      vp
      ? set_memo(CHK_VP, curr_ix, *ix, vp)
      : set_memo(CHK_VP, curr_ix, *ix=curr_ix, vp));
}  // parse_vp


EnglishConjunction* parse_conj(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishConjunction* conj =
      static_cast<EnglishConjunction*>(get_memo(CHK_CONJUNCTION, ix));
  if (conj) return conj;

  Word* word = &words[*ix];
  string surface = word->surface();
  string surface_ = strlower(surface);
  printf("   - parse_conj([%s,..], %d)...\n", surface.c_str(), *ix);

  if ((*ix == 0 && surface_ == "that")
      || surface_ == "an") {
    *ix = curr_ix;
    return NULL;
  }

  if (word->pos_canbe("接続")) {
    return new EnglishConjunction(words[(*ix)++]);
  }

  *ix = curr_ix;
  return NULL;
}  // parse_conj


EnglishSentence* parse_sentence(vector<Word>& words, unsigned int *ix) {
  if (*ix == words.size()) return NULL;
  int curr_ix = *ix;
  EnglishSentence* st =
      static_cast<EnglishSentence*>(get_memo(CHK_SENTENCE, ix));
  if (st) return st;

  do {
    EnglishNP* np = parse_np(words, ix);
    if (!np) break;

    printf("NP found; then searchin' for vp...(%d/%d)\n", *ix, (int)words.size());
    printf("40\n");
    if (*ix == words.size()) break;
    printf("41\n");

    if (EnglishVP* vp = parse_vp(words, ix)) {
      st = new EnglishSentence(np, vp);
      cout << ANSI_BOLD_ON << "[S] " << st->surface() << ANSI_BOLD_OFF << endl;
      st->dump();
    } else {
      // delete np;
    }
  } while (false);

  if (!st)
    cout << ANSI_ITALICS_ON << "[!S] " << ANSI_ITALICS_OFF << endl;

  return static_cast<EnglishSentence*>(
      st
      ? set_memo(CHK_SENTENCE, curr_ix, *ix, st)
      : set_memo(CHK_SENTENCE, curr_ix, *ix=curr_ix, NULL));
}  // parse_sentence


std::vector<WObj*> parse(std::vector<Word>& words) {
  printf(" - parse(%d words)...\n", (int)words.size());
  clear_memo();

  vector<WObj*> objs;
  for (unsigned int ix = 0, c = words.size(); ix < c; ) {
    printf("\n[LOOP] ix=%d ... ", (int)ix);

    int curr_ix = ix;
    cout << ANSI_UNDERLINE_ON;
    for (unsigned int j = 0; j < ix; ++j) {
      cout << words[j].surface() << " ";
    }
    cout << ANSI_FGCOLOR_RED;
    cout << ANSI_BOLD_ON << words[ix].surface() << ANSI_BOLD_OFF;
    cout << ANSI_FGCOLOR_BLUE;
    for (unsigned int j = ix+1; j < c; ++j) {
      cout << " " << words[j].surface();
    }
    cout << ANSI_FGCOLOR_DEFAULT << ANSI_UNDERLINE_OFF << endl;


    WObj* obj = NULL;
    while (!obj) {
      if (ix == words.size()) break;
      int ixkept = ix;

      string surface = words[ix].surface(), surface_ = strlower(surface);
      printf(ANSI_FGCOLOR_GREEN "\n[parse] ix=%d surface=\"%s\" ... " ANSI_FGCOLOR_DEFAULT,
             ix, surface.c_str());

      printf(ANSI_FGCOLOR_GREEN "conj... " ANSI_FGCOLOR_DEFAULT);
      obj = parse_conj(words, &ix);
      if (obj) {
        printf(" ! (%d-%d, [%s]) => conj. %s\n",
               curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
        break;
      }
      if (ix != ixkept) printf("XXX ix %d != ix' %d\n", ix, ixkept);

      printf(ANSI_FGCOLOR_GREEN "sentence... " ANSI_FGCOLOR_DEFAULT);
      obj = parse_sentence(words, &ix);
      if (obj) {
        printf(" ! (%d-%d, [%s]) => S %s\n",
               curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
        break;
      }
      if (ix != ixkept) printf("XXX ix %d != ix' %d\n", ix, ixkept);

      printf(ANSI_FGCOLOR_GREEN "np... " ANSI_FGCOLOR_DEFAULT);
      obj = parse_np(words, &ix);
      if (obj) {
        printf(" ! (%d-%d, [%s]) => NP %s\n",
               curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
        break;
      }
      if (ix != ixkept) printf("XXX ix %d != ix' %d\n", ix, ixkept);

      printf(ANSI_FGCOLOR_GREEN "vp... " ANSI_FGCOLOR_DEFAULT);
      obj = parse_vp(words, &ix);
      if (obj) {
        printf(" ! (%d-%d, [%s]) => VP %s\n",
               curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
        break;
      }
      if (ix != ixkept) printf("XXX ix %d != ix' %d\n", ix, ixkept);

      printf(ANSI_FGCOLOR_GREEN "pp... " ANSI_FGCOLOR_DEFAULT);
      obj = parse_pp(words, &ix);
      if (obj) {
        printf(" ! (%d-%d, [%s]) => PP %s\n",
               curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
        break;
      }
      if (ix != ixkept) printf("XXX ix %d != ix' %d\n", ix, ixkept);

      printf(ANSI_FGCOLOR_GREEN "raw word... " ANSI_FGCOLOR_DEFAULT);
      obj = new Word(words[ix++]);  // const_cast<Word*>(&words[ix++]);
      if (obj) {
        // printf(" ! (%d-%d, [%s]) => ** %s\n",
        //        curr_ix, ix-1, surface.c_str(), obj->surface().c_str());
        printf(" ! (%d-%d, [%s]) => ** %p %s\n",
               curr_ix, ix-1, surface.c_str(), obj, obj->surface().c_str());
        break;
      }
      if (ix != ixkept) printf("XXX ix %d != ix' %d\n", ix, ixkept);

      printf(ANSI_FGCOLOR_GREEN " OBJ NOT FOUND! (%d-%d, [%s]) => ??\n"  ANSI_FGCOLOR_DEFAULT,
             curr_ix, ix-1, surface.c_str());
    }  // while (!obj)
    printf("\n");

    // cout << " // now pushing back (" << obj->surface() << ")" << endl;
    if (obj) objs.push_back(obj);
  }

  // clear_memo();

  return objs;
}
