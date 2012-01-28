// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#ifndef SANDBOX_ENGLISHGRAMMAR_H_
#define SANDBOX_ENGLISHGRAMMAR_H_

#include <string>
#include <vector>

#include "sandbox/Word.h"

class EObj : public WObj {
 public:
  explicit EObj(Word *word);
  EObj(Word *word, std::string pos);
  ~EObj() {}

  virtual std::string surface() const {
    return word_->surface();
  }
  std::vector<std::string> pos() const {
    return std::vector<std::string>(1, the_pos_);
  }

  virtual std::string translate() {
    return word_->translate(the_pos_);
  }

  // virtual void dump(int indent = 0) {
  //   for (int i=0; i<indent; ++i) putchar(' ');
  //   printf("%s\n", surface().c_str());
  // }

  // virtual std::string translate(std::string pos) {
  //   return word_->translate(pos);
  // }

 protected:
  Word *word_;
  std::string the_pos_;
};

//
// Verb - 動詞
//
class EnglishVerb : public EObj {
 public:
  explicit EnglishVerb(Word *word);
  ~EnglishVerb() {}

  std::string translate();
  void dump(int indent = 0);
};

//
// Noun - 名詞
//
class EnglishEntity : public EObj {
 public:
  // explicit EnglishModifier(Word *word);
  EnglishEntity(Word *word, std::string pos);
  ~EnglishEntity() {}
};

class EnglishNoun : public EnglishEntity {
 public:
  explicit EnglishNoun(Word *word);
  ~EnglishNoun() {}

  std::string translate();
  void dump(int indent = 0);
};

//
// Name - 固有名詞
//
class EnglishName : public EnglishEntity {
 public:
  explicit EnglishName(Word *word);
  ~EnglishName() {}

  std::string translate();
  void dump(int indent = 0);
};

//
// Pronoun - 代名詞 { I you he/she/it we they; me him/her, .. }
//
class EnglishPronoun : public EnglishEntity {
 public:
  explicit EnglishPronoun(Word *word);
  ~EnglishPronoun() {}

  std::string translate() { return word_->translate("代名"); }
  void dump(int indent = 0);
};

class EnglishModifier : public EObj {
 public:
  // explicit EnglishModifier(Word *word);
  EnglishModifier(Word *word, std::string pos);
  ~EnglishModifier() {}

  // std::string translate() { return word_->translate(); }
  // void dump(int indent = 0);
};

//
// Determiner - 冠詞 { the, a/an }
//
class EnglishDeterminer : public EnglishModifier {
 public:
  explicit EnglishDeterminer(Word *word);
  ~EnglishDeterminer() {}

  std::string translate() { return ""; }  // return word_->translate(); }
  void dump(int indent = 0);
};

//
// Adjective - 形容詞
//
class EnglishAdjective : public EnglishModifier {
 public:
  explicit EnglishAdjective(Word *word);
  ~EnglishAdjective() {}

  std::string translate() { return word_->translate("形"); }
  void dump(int indent = 0);
};

//
// Adverb - 副詞
//
class EnglishAdverb : public EObj {
 public:
  explicit EnglishAdverb(Word *word);
  ~EnglishAdverb() {}

  std::string translate();  // { return word_->translate("副"); }
  void dump(int indent = 0);
};


//
// Preposition - 前置詞
//
class EnglishPreposition : public EObj {
 public:
  explicit EnglishPreposition(Word *word);
  ~EnglishPreposition() {}

  std::string translate();
  void dump(int indent = 0);
};

//
// Conjunction - 接続詞
//
class EnglishConjunction : public EObj {
 public:
  explicit EnglishConjunction(Word *word);
  ~EnglishConjunction() {}

  std::string translate();
  void dump(int indent = 0);
};



class EnglishNP;

class EnglishPP : public WObj {
 public:
  EnglishPP(EnglishPreposition *prep, EnglishNP *np);
  ~EnglishPP() {}

  std::string surface() const;
  std::string translate();
  void dump(int indent = 0);

 private:
  EnglishPreposition *prep_;
  EnglishNP *np_;
};


class EnglishNP : public WObj {
 public:
  explicit EnglishNP(EnglishEntity* entity,
                     std::vector<EnglishModifier*> modifiers);
  ~EnglishNP() {}

  void append_entity(EnglishEntity* entity);
  void add_pp(EnglishPP *pp);

  std::string surface() const;
  std::string translate();
  void dump(int indent = 0);

 private:
  std::vector<EnglishModifier*> modifiers_;
  std::vector<EnglishEntity*> entities_;
  std::vector<EnglishPP*> pps_;
};


class EnglishAP : public WObj {
 public:
  explicit EnglishAP(EnglishAdjective* adj);
  ~EnglishAP() {}

  void append_adjective(EnglishAdjective* adj);

  std::string surface() const;
  std::string translate();
  void dump(int indent = 0);

 private:
  std::vector<EnglishAdjective*> adjectives_;
};


class EnglishVP : public WObj {
 public:
  explicit EnglishVP(EnglishVerb *verb);
  ~EnglishVP() {}

  void append_verb(EnglishVerb* verb);
  void add_np(EnglishNP* np);
  void add_pp(EnglishPP* pp);
  void add_ap(EnglishAP* ap);
  void add_adverb(EnglishAdverb* adv);

  std::string surface() const;
  std::string translate();
  void dump(int indent = 0);

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
  ~EnglishSentence() {}

  std::string surface() const;
  std::string translate();
  void dump(int indent = 0);

 private:
  EnglishNP *np_;
  EnglishVP *vp_;
};

#endif  // SANDBOX_ENGLISHGRAMMAR_H_
