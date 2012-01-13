#ifndef PDIC_DATAFIELD_H
#define PDIC_DATAFIELD_H

class PDICDatafield {
 public:
  int            start_pos;
  int            field_length;

  unsigned char *entry_word;
  int            entry_word_size;
  int            entry_word_attrib;
  unsigned char *entry_index;
  int            entry_index_size;
  int            charcode;
  unsigned char *_entry_word_utf8;
  unsigned char *_tabsep;

  unsigned char *data;
  int            data_size;
  unsigned char *_jword_utf8;

  bool           is_retained;

 public:
  PDICDatafield(int start_pos, int field_length,
                unsigned char *entry_word, int entry_word_size, int entry_word_attrib,
                int charcode, unsigned char *data, int data_size);
  ~PDICDatafield();

 public:
  void retain();

  unsigned char *entry_word_utf8();
  unsigned char *jword_utf8();
};

#endif
