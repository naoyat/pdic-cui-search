// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_ENGLISHGRAMMAR_H_
#define SANDBOX_ENGLISHGRAMMAR_H_

#include <string>
#include <vector>

#include "sandbox/Word.h"

//
// Verb - 動詞
//
class EnglishVerb : public Word {
 public:
  EnglishVerb(Word& word);
  virtual ~EnglishVerb();

  virtual const char* type() { return "EnglishVerb"; }

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);

  virtual bool isBeVerb() { return false; }
};


class EnglishBe : public EnglishVerb {
 public:
  EnglishBe(Word& word);
  virtual ~EnglishBe();

  virtual const char* type() { return "EnglishBe"; }

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);

  bool isBeVerb() { return true; }
};


//
// EnglishEntity
//
class EnglishEntity : public Word {
 public:
  // explicit EnglishModifier(Word *word);
  EnglishEntity(Word& word, const char *pos);
  virtual ~EnglishEntity();

  virtual const char* type() { return "EnglishEntity"; }
};


//
// EnglishNoun
//
class EnglishNoun : public EnglishEntity {
 public:
  EnglishNoun(Word& word);
  virtual ~EnglishNoun();

  virtual const char* type() { return "EnglishNoun"; }

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};


//
// Name - 固有名詞
//
class EnglishName : public EnglishEntity {
 public:
  EnglishName(Word& word);
  virtual ~EnglishName();

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};


//
// Pronoun - 代名詞 { I you he/she/it we they; me him/her, .. }
//
class EnglishPronoun : public EnglishEntity {
 public:
  EnglishPronoun(Word& word);
  virtual ~EnglishPronoun();

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};


//
// EnglishModifier
//
class EnglishModifier : public Word {
 public:
  // explicit EnglishModifier(Word *word);
  EnglishModifier(Word& word, const char *pos);
  virtual ~EnglishModifier();
};


//
// Determiner - 冠詞 { the, a/an }
//
class EnglishDeterminer : public EnglishModifier {
 public:
  EnglishDeterminer(Word& word);
  virtual ~EnglishDeterminer();

  std::string translate() { return ""; }  // return word_->translate(); }
  // virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};


//
// Adjective - 形容詞
//
class EnglishAdjective : public EnglishModifier {
public:
  EnglishAdjective(Word& word);
  virtual ~EnglishAdjective();

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};


//
// Adverb - 副詞
//
class EnglishAdverb : public Word {
 public:
  EnglishAdverb(Word& word);
  virtual ~EnglishAdverb();

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};


//
// Preposition - 前置詞
//
class EnglishPreposition : public Word {
 public:
  EnglishPreposition(Word& word);
  virtual ~EnglishPreposition();

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};


//
// Conjunction - 接続詞
//
class EnglishConjunction : public Word {
 public:
  EnglishConjunction(Word& word);
  virtual ~EnglishConjunction();

  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);
};



class EnglishNP;

class EnglishPP : public WObj {
 public:
  EnglishPP(EnglishPreposition *prep, EnglishNP *np);
  virtual ~EnglishPP();

  virtual const char* type() { return "EnglishPP"; }

  std::string surface();
  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);

 private:
  EnglishPreposition *prep_;
  EnglishNP *np_;
};


class EnglishNP : public WObj {
 public:
  explicit EnglishNP(EnglishEntity* entity,
                     std::vector<EnglishModifier*> modifiers);
  virtual ~EnglishNP();

  virtual const char* type() { return "EnglishNP"; }

  void append_entity(EnglishEntity* entity);
  void add_pp(EnglishPP *pp);

  std::string surface();
  virtual std::string translate();
  // virtual std::string translate(const char *pos);
  virtual void dump(int indent = 0);

 private:
  std::vector<EnglishModifier*> modifiers_;
  std::vector<EnglishEntity*> entities_;
  std::vector<EnglishPP*> pps_;
};


class EnglishAP : public WObj {
 public:
  explicit EnglishAP(EnglishAdjective* adj);
  virtual ~EnglishAP();

  virtual const char* type() { return "EnglishAP"; }

  void append_adjective(EnglishAdjective* adj);

  std::string surface();
  virtual std::string translate();
  // virtual std::string translate_with_pos(const char *pos);
  virtual void dump(int indent = 0);

 private:
  std::vector<EnglishAdjective*> adjectives_;
};


class EnglishVP : public WObj {
 public:
  explicit EnglishVP(EnglishVerb *verb);
  virtual ~EnglishVP();

  virtual const char* type() { return "EnglishVP"; }

  void append_verb(EnglishVerb* verb);
  void add_np(EnglishNP* np);
  void add_pp(EnglishPP* pp);
  void add_ap(EnglishAP* ap);
  void add_adverb(EnglishAdverb* adv);

  std::string surface();
  virtual std::string translate();
  // virtual std::string translate_with_pos(const char *pos);
  virtual void dump(int indent = 0);

 private:
  std::vector<EnglishVerb*> verbs_;
  EnglishNP *np_;
  EnglishAP *ap_;
  std::vector<EnglishPP*> pps_;
  std::vector<EnglishAdverb*> adverbs_;
};


class EnglishSentence : public WObj {
 public:
  EnglishSentence(EnglishNP* np, EnglishVP* vp);
  virtual ~EnglishSentence();

  EnglishSentence(const EnglishSentence& st);

  virtual const char* type() { return "EnglishSentence"; }

  std::string surface();
  virtual std::string translate();
  // virtual std::string translate_with_pos(const char *pos);
  virtual void dump(int indent = 0);

 private:
  EnglishNP *np_;
  EnglishVP *vp_;
};

#endif  // SANDBOX_ENGLISHGRAMMAR_H_
