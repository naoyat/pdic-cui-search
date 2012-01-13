#include "PDICDatafield.h"

#include <cstdlib>
#include <cstring>

#include "bocu1.h"
#include "charcode.h"
#include "utf8.h"
#include "util.h"
#include "types.h"

PDICDatafield::PDICDatafield(int start_pos,
                             int field_length,
                             byte *entry_word,
                             int entry_word_size,
                             int entry_word_attrib,
                             int charcode,
                             byte *data,
                             int data_size,
                             bool v6index,
                             Criteria *criteria)
{
  this->start_pos = start_pos;
  this->field_length = field_length;
  this->v6index = v6index;
  this->criteria = criteria;

  _tabsep = (byte *)strchr((char *)entry_word, '\t');
  if (_tabsep) {
    this->entry_index = entry_word;
    this->entry_index_size = (int)(_tabsep - entry_word);
    //*_tabsep = 0;
    this->entry_word = _tabsep + 1;
    this->entry_word_size  = entry_word_size - this->entry_index_size - 1;
  } else {
    this->entry_word = this->entry_index = entry_word;
    this->entry_word_size = this->entry_index_size = entry_word_size;
  }
  this->entry_word_attrib   = entry_word_attrib;
  this->charcode            = charcode;
  //printf("[%s (%d), %d], %d\n", entry_word_utf8(), entry_word_attrib, entry_word_size, charcode);

  this->data                = data;
  this->data_size           = data_size;

  this->is_retained         = false;

  this->_entry_word_utf8    = NULL;
  this->_jword_utf8         = NULL;
}

PDICDatafield::~PDICDatafield()
{
  if (is_retained) {
    free((void *)entry_word);
    free((void *)data);
  }
  if (_entry_word_utf8) free((void *)_entry_word_utf8);
  if (_jword_utf8) free((void *)_jword_utf8);
}

void
PDICDatafield::retain()
{
  if (!is_retained) {
    entry_word  = clone_cstr(entry_word, 0, false);
    data        = clone_cstr(data, data_size, false);
    is_retained = true;
  }
}

byte *
PDICDatafield::entry_word_utf8()
{
  if (!_entry_word_utf8) {
    switch (charcode) {
      case CHARCODE_BOCU1:
        _entry_word_utf8 = bocu1_to_utf8(entry_word, entry_word_size);
        break;
      case CHARCODE_SHIFTJIS:
        _entry_word_utf8 = sjis_to_utf8(entry_word, entry_word_size);
        break;
      default:
        break;
    }
    if (!_entry_word_utf8) return entry_word;
  }
  return _entry_word_utf8;
}

byte *
PDICDatafield::jword_utf8()
{
  if (!_jword_utf8) {
    byte *jword = data;
    int jword_size = 0;
    if (entry_word_attrib & 0x10) {
      jword_size = strlen((char *)jword);
      //ofs += jword_datalength + 1;
      //printf("\n <%d: %d %d %02x>", field_id,  field_length, compress_length, entry_word_attrib);
    } else {
      jword_size = data_size;
    }

    switch (charcode) {
      case CHARCODE_BOCU1:
        _jword_utf8 = bocu1_to_utf8(jword, jword_size);
        break;
      case CHARCODE_SHIFTJIS:
        _jword_utf8 = sjis_to_utf8(jword, jword_size);
        break;
      default:
        _jword_utf8 = clone_cstr(jword, jword_size, false);
        break;
    }
  }
  return _jword_utf8;
}
