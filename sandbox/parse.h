// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_PARSE_H_
#define SANDBOX_PARSE_H_

#include <string>
#include <vector>

#include "sandbox/Word.h"
#include "util/types.h"

class WObj;
class Word;
class EnglishEntity;
class EnglishModifier;
class EnglishNP;
class EnglishPP;
class EnglishBe;
class EnglishVerb;
class EnglishAdverb;
class EnglishAP;
class EnglishVP;
class EnglishConjunction;
class EnglishSentence;

// lookup_result e_just(byte *needle);

EnglishEntity* parse_entity(std::vector<Word>& words, unsigned int *ix);
EnglishModifier* parse_modifier(std::vector<Word>& words, unsigned int *ix);
EnglishNP* parse_np(std::vector<Word>& words, unsigned int *ix);
EnglishPP* parse_pp(std::vector<Word>& words, unsigned int *ix);
EnglishBe* parse_be(std::vector<Word>& words, unsigned int *ix);
EnglishVerb* parse_verb(std::vector<Word>& words, unsigned int *ix);
EnglishAdverb* parse_adverb(std::vector<Word>& words, unsigned int *ix);
EnglishAP* parse_ap(std::vector<Word>& words, unsigned int *ix);
EnglishVP* parse_vp(std::vector<Word>& words, unsigned int *ix);
EnglishConjunction* parse_conj(std::vector<Word>& words, unsigned int *ix);
EnglishSentence* parse_sentence(std::vector<Word>& words, unsigned int *ix);

std::vector<WObj> parse(std::vector<Word>& words);

#endif  // SANDBOX_PARSE_H_
