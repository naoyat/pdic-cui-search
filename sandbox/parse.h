// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_PARSE_H_
#define SANDBOX_PARSE_H_

#include <string>
#include <vector>

#include "util/types.h"

class WObj;
class Word;
class EnglishEntity;
class EnglishModifier;
class EnglishNP;
class EnglishPP;
class EnglishVP;

// lookup_result e_just(byte *needle);

EnglishEntity* parse_entity(const std::vector<Word*>& words, unsigned int *ix);
EnglishModifier* parse_modifier(const std::vector<Word*>& words,
                                unsigned int *ix);
EnglishNP* parse_np(const std::vector<Word*>& words, unsigned int *ix);
EnglishPP* parse_pp(const std::vector<Word*>& words, unsigned int *ix);
EnglishVP* parse_vp(const std::vector<Word*>& words, unsigned int *ix);

std::vector<WObj*> parse(const std::vector<Word*>& words);

#endif  // SANDBOX_PARSE_H_
