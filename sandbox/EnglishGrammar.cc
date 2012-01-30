// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "sandbox/EnglishGrammar.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "sandbox/Word.h"
#include "util/ansi_color.h"
#include "util/stlutil.h"

using namespace std;

//
// EnglishVerb
//
EnglishVerb::EnglishVerb(const Word& word) : Word(word, "[V]") {
  printf("EnglishVerb(copy:word);\n");
}

EnglishVerb::~EnglishVerb() {
  printf("~EnglishVerb();\n");
}

std::string EnglishVerb::translate() {
  std::string tr;
  if (pos_canbe("動")) tr = translate_with_pos("動");
  if (pos_canbe("他動")) tr = translate_with_pos("他動");
  if (pos_canbe("自動")) tr = translate_with_pos("自動");
  if (pos_canbe("助動")) tr = translate_with_pos("助動");
  if (tr == "") tr = surface();

  RE2::GlobalReplace(&tr, "^を", "");
  return tr;
}

void EnglishVerb::dump(int indent) {
  cout << string(indent, ' ') << "V: " << surface() << endl;
}


//
// ENglishBe
//
EnglishBe::EnglishBe(const Word& word) : EnglishVerb(word) {
  printf("EnglishBe(copy:word);\n");
}

EnglishBe::~EnglishBe() {
  printf("~EnglishBe();\n");
}

std::string EnglishBe::translate() {
  return " = ";
}

void EnglishBe::dump(int indent) {
  cout << string(indent, ' ') << "V(be): " << surface() << endl;
}


//
// EnglishEntity
//
EnglishEntity::EnglishEntity(const Word& word, const char *pos) : Word(word, pos) {
  printf("EnglishEntity(copy:word; pos=%s);\n", pos);
}

EnglishEntity::~EnglishEntity() {
  printf("~EnglishEntity();\n");
}


//
// ENglishNoun
//
EnglishNoun::EnglishNoun(const Word& word) : EnglishEntity(word, "[N]") {
  printf("EnglishNoun(copy:word);\n");
}

EnglishNoun::~EnglishNoun() {
  printf("~EnglishNoun();\n");
}

std::string EnglishNoun::translate() {
  string tr = translate_with_pos("名");
  if (tr != "") return tr;
  return surface();
}
void EnglishNoun::dump(int indent) {
  cout << string(indent, ' ') << "N: " << surface() << endl;
}


//
// EnglishName
//
EnglishName::EnglishName(const Word& word) : EnglishEntity(word, "[Name]") {
  printf("EnglishName(copy:word);\n");
}

EnglishName::~EnglishName() {
  printf("~EnglishName();\n");
}

std::string EnglishName::translate() {
  string tr = translate_with_pos("名前");
  if (tr != "") return tr;
  return surface();
}
void EnglishName::dump(int indent) {
  cout << string(indent, ' ') << "Name: " << surface() << endl;
}


//
// EnglishPronoun
//
EnglishPronoun::EnglishPronoun(const Word& word)
    : EnglishEntity(word, "[Pron]") {
  printf("EnglishPronoun(copy:word);\n");
}

EnglishPronoun::~EnglishPronoun() {
  printf("~EnglishPronoun();\n");
}

std::string EnglishPronoun::translate() {
  string tr = translate_with_pos("代名");
  return tr;
}

void EnglishPronoun::dump(int indent) {
  cout << string(indent, ' ') << "Pron: " << surface() << endl;
}


//
// EnglishModifier
//
EnglishModifier::EnglishModifier(const Word& word, const char *pos)
    : Word(word, pos) {
  printf("EnglishModifier(copy:word; pos=%s);\n", pos);
}

EnglishModifier::~EnglishModifier() {
  printf("~EnglishModifier();\n");
}


//
// EnglishDeterminer
//
EnglishDeterminer::EnglishDeterminer(const Word& word)
    : EnglishModifier(word, "[Det]") {
  printf("EnglishDeterminer(copy:word);\n");
}

EnglishDeterminer::~EnglishDeterminer() {
  printf("~EnglishDeterminer();\n");
}

void EnglishDeterminer::dump(int indent) {
  cout << string(indent, ' ') << "D: " << surface() << endl;
}


//
// EnglishAdjective
//
EnglishAdjective::EnglishAdjective(const Word& word)
    : EnglishModifier(word, "[Adj]") {
  printf("EnglishAdjective(copy:word);\n");
}

EnglishAdjective::~EnglishAdjective() {
  printf("~EnglishAdjective();\n");
}

std::string EnglishAdjective::translate() {
  string tr = translate_with_pos("形");
  return tr;
}

void EnglishAdjective::dump(int indent) {
  cout << string(indent, ' ') << "A: " << surface() << endl;
}


//
// EnglishAdverb
//
EnglishAdverb::EnglishAdverb(const Word& word) : Word(word, "[Adv]") {
  printf("EnglishAdverb(copy:word);\n");
}

EnglishAdverb::~EnglishAdverb() {
  printf("~EnglishAdverb();\n");
}

std::string EnglishAdverb::translate() {
  std::string tr = translate_with_pos("副");
  return tr;
}
void EnglishAdverb::dump(int indent) {
  cout << string(indent, ' ') << "Adv: " << surface() << endl;
}


//
// EnglishPreposition
//
EnglishPreposition::EnglishPreposition(const Word& word) : Word(word, "[Prep]") {
  printf("EnglishPreposition(copy:word);\n");
}

EnglishPreposition::~EnglishPreposition() {
  printf("~EnglishPreposition();\n");
}

std::string EnglishPreposition::translate() {
  std::string tr = translate_with_pos("前");
  if (tr.substr(0, 3) == "\xef\xbd\x9e")  // U+FF5E, "〜" in MS932
    return tr.substr(3);
  else
    return tr;
}
void EnglishPreposition::dump(int indent) {
  cout << string(indent, ' ') << "Prep: " << surface() << endl;
}


//
// EnglishConjunction
//
EnglishConjunction::EnglishConjunction(const Word& word) : Word(word, "[Conj]") {
  printf("EnglishConjunction(copy:word);\n");
}

EnglishConjunction::~EnglishConjunction() {
  printf("~EnglishConjunction();\n");
}

std::string EnglishConjunction::translate() {
  std::string tr = translate_with_pos("接続");
  if (tr != "") return tr;
  // if (tr.substr(0,3) == "\xef\xbd\x9e")  // U+FF5E, "〜" in MS932
  // return tr.substr(3);
  return surface();
}
void EnglishConjunction::dump(int indent) {
  cout << string(indent, ' ') << "Conj:" << surface() << endl;
}


//
// PP
//
EnglishPP::EnglishPP(EnglishPreposition *prep, EnglishNP *np) {
  this->prep_ = prep;
  this->np_ = np;
  this->pos_ = std::vector<std::string>(1, "[PP]");
  printf("EnglishPP(*prep, *np);\n");
}

EnglishPP::~EnglishPP() {
  printf("~EnglishPP();\n");
}

std::string EnglishPP::surface() const {
  return "(" + prep_->surface() + " " + np_->surface() + ")";
}

std::string EnglishPP::translate() {
  std::stringstream ss;
  ss << np_->translate();
  ss << prep_->translate();
  return ss.str();
}

void EnglishPP::dump(int indent) {
  cout << string(indent, ' ') << "PP:" << endl;
  prep_->dump(indent + 2);
  np_->dump(indent + 2);
}


//
// NP
//
EnglishNP::EnglishNP(EnglishEntity *entity,
                     vector<EnglishModifier*> modifiers) {
  this->modifiers_.assign(modifiers.begin(), modifiers.end());
  this->entities_.push_back(entity);
  this->pps_.clear();
  this->pos_ = std::vector<std::string>(1, "[NP]");
  printf("EnglishNP(*entity, *modifiers);\n");
}

EnglishNP::~EnglishNP() {
  printf("~EnglishNP();\n");
}

void EnglishNP::append_entity(EnglishEntity* entity) {
  this->entities_.push_back(entity);
}

void EnglishNP::add_pp(EnglishPP *pp) {
  this->pps_.push_back(pp);
}

std::string EnglishNP::surface() const {
  std::stringstream ss;
  ss << "(";
  traverse(modifiers_, it) {
    ss << (*it)->surface() << " ";
  }
  ss << "<";
  traverse(entities_, it) {
    if (it != entities_.begin()) ss << " ";
    ss << (*it)->surface();
  }
  ss << ">";
  traverse(pps_, it) {
    ss << " " << (*it)->surface();
  }

  ss << ")";
  return ss.str();
}

std::string EnglishNP::translate() {
  std::stringstream ss;

  if (modifiers_.size() > 0 || pps_.size() > 0) {
    // ss << "{";
    traverse(pps_, it) {
      ss << (*it)->translate();
    }
    traverse(modifiers_, it) {
      ss << (*it)->translate();
    }
    // ss << "}";
  }
  traverse(entities_, it) {
    if (it != entities_.begin()) ss << " ";
    ss << (*it)->translate();
  }

  return ss.str();
}

void EnglishNP::dump(int indent) {
  cout << string(indent, ' ') << "NP:" << endl;
  traverse(modifiers_, it) {
    (*it)->dump(indent + 2);
  }
  cout << string(indent + 2, ' ') << "N:";
  traverse(entities_, it) {
    // if (it != entities_.begin())
    cout << " ";
    cout << (*it)->surface();
  }
  cout << endl;
  traverse(pps_, it) {
    (*it)->dump(indent + 2);
  }
}


//
// AP
//
EnglishAP::EnglishAP(EnglishAdjective* adj) {
  this->adjectives_.push_back(adj);
  this->pos_ = std::vector<std::string>(1, "[AP]");
  printf("EnglishAP(*adj);\n");
}

EnglishAP::~EnglishAP() {
  printf("~EnglishAP();\n");
}

void EnglishAP::append_adjective(EnglishAdjective* adj) {
  this->adjectives_.push_back(adj);
}

std::string EnglishAP::surface() const {
  std::stringstream ss;
  traverse(adjectives_, it) {
    if (it != adjectives_.begin())
      ss << " ";
    ss << (*it)->surface();
  }
  return ss.str();
}
std::string EnglishAP::translate() {
  std::stringstream ss;
  traverse(adjectives_, it) {
    ss << (*it)->translate();  // << "+";
  }
  return ss.str();
}

void EnglishAP::dump(int indent) {
  cout << string(indent, ' ') << "AP:" << endl;
  traverse(adjectives_, it) {
    (*it)->dump(indent + 2);
  }
}


//
// VP
//
EnglishVP::EnglishVP(EnglishVerb *verb) {
  this->verbs_.push_back(verb);
  this->adverbs_.clear();
  this->np_ = NULL;
  this->ap_ = NULL;
  this->pps_.clear();
  this->pos_ = std::vector<std::string>(1, "[VP]");
  printf("EnglishVP(*verb);\n");
}

EnglishVP::~EnglishVP() {
  printf("~EnglishVP();\n");
}

void EnglishVP::append_verb(EnglishVerb* verb) {
  this->verbs_.push_back(verb);
}

void EnglishVP::add_np(EnglishNP* np) {
  this->np_ = np;
}

void EnglishVP::add_ap(EnglishAP* ap) {
  this->ap_ = ap;
}

void EnglishVP::add_pp(EnglishPP* pp) {
  this->pps_.push_back(pp);
}

void EnglishVP::add_adverb(EnglishAdverb* adv) {
  this->adverbs_.push_back(adv);
}

std::string EnglishVP::surface() const {
  std::stringstream ss;

  if (verbs_[0]->isBeVerb()) {
    // V+C => C です
  } else {
    // V+O => O を V する
  }

  ss << "(";
  traverse(adverbs_, it) {
    ss << (*it)->surface() << " ";
  }
  ss << "<";
  traverse(verbs_, it) {
    if (it != verbs_.begin()) ss << " ";
    ss << (*it)->surface();
  }
  ss << ">";

  if (verbs_[0]->isBeVerb()) {
    if (ap_) {
      ss << " " << ap_->surface();
    }
  }

  if (np_) ss << " " << np_->surface();

  traverse(pps_, it) {
    ss << " " << (*it)->surface();
  }
  ss << ")";
  return ss.str();
}

std::string EnglishVP::translate() {
  std::stringstream ss;

  if (verbs_[0]->isBeVerb()) {
    ss << "-は ";
    if (ap_) {
      ss << ap_->translate();
    }
    if (np_) {
      ss << np_->translate();
    }
    ss << "です";
  } else {
    if (np_) {
      ss << np_->translate();
    }
    ss << "を";
  }

  if (adverbs_.size() > 0) {
    ss << " adv#{";
    traverse(adverbs_, it) {
      ss << (*it)->translate();
    }
    ss << "}";
  }

  if (pps_.size() > 0) {
    ss << " pp+#{";
    traverse(pps_, it) {
      ss << (*it)->translate();
    }
    ss << "}";
  }

  string surface_ = strlower(verbs_[0]->surface());
  if (verbs_.size() == 1) {
    if (verbs_[0]->isBeVerb()) {
      //
    } else {
      ss << verbs_[0]->translate();
    }
  } else {
    if (surface_ == "don't") {
      ss << "(not-";
      traverse(verbs_, it) {
        ss << (*it)->translate();
      }
      ss << ")";
    } else if (surface_ == "was" || surface_ == "were") {
      ss << "(";
      traverse(verbs_, it) {
        ss << (*it)->translate();
      }
      ss << ")された";
    } else {
      traverse(verbs_, it) {
        ss << (*it)->translate();
      }
    }
  }

  // string tr;
  // if (np_) tr += np_->translate() + aux;
  return ss.str();
}

void EnglishVP::dump(int indent) {
  cout << string(indent, ' ') << "VP:" << endl;

  /*
  verb_->dump(indent + 2);
  traverse(additional_verbs_, it) {
    (*it)->dump(indent + 2);
  }
  */
  if (verbs_[0]->isBeVerb())
    cout << string(indent + 2, ' ') << "V<be>:";
  else
    cout << string(indent + 2, ' ') << "V:";
  traverse(verbs_, it) {
    cout << " " << (*it)->surface();
  }
  cout << endl;
  if (ap_) {
    ap_->dump(indent + 2);
  } else {
    if (np_) np_->dump(indent + 2);
  }
  traverse(adverbs_, it) {
    (*it)->dump(indent + 2);
  }
  traverse(pps_, it) {
    (*it)->dump(indent + 2);
  }
}


//
// EnglishSentence
//
EnglishSentence::EnglishSentence(EnglishNP* np, EnglishVP* vp) {
  this->np_ = np;
  this->vp_ = vp;
  this->pos_ = std::vector<std::string>(1, "[S]");
  printf("EnglishSentence(*np, *vp);\n");
}

EnglishSentence::~EnglishSentence() {
  printf("~EnglishSentence();\n");
}

std::string EnglishSentence::surface() const {
  return np_->surface() + " " + vp_->surface();
}

std::string EnglishSentence::translate() {
  // return "("+np_->translate() + ")" が "(" + vp_->translate() +")";
  return "("+np_->translate() + ")_(" + vp_->translate() +")";
}

void EnglishSentence::dump(int indent) {
  cout << string(indent, ' ') << "S:" << endl;
  np_->dump(indent + 2);
  vp_->dump(indent + 2);
}
