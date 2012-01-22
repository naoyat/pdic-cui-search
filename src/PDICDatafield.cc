// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include "./PDICDatafield.h"

#include <stdlib.h>
#include <string.h>

#include "./bocu1.h"
#include "./charcode.h"
#include "./Dict.h"
#include "./stlutil.h"
#include "./types.h"
#include "./utf8.h"
#include "./util.h"


PDICDatafield::PDICDatafield(int start_pos,
                             int field_length,
                             byte *entry_word,
                             int entry_word_size,
                             int entry_word_attrib,
                             int charcode,
                             byte *data,
                             int data_size,
                             bool v6index,
                             Criteria* criteria) {
  this->start_pos = start_pos;
  this->field_length = field_length;
  this->v6index = v6index;
  this->criteria = criteria;

  _tabsep = reinterpret_cast<byte*>(
      strchr(reinterpret_cast<char*>(entry_word), '\t'));

  if (_tabsep) {
    this->entry_index = entry_word;
    this->entry_index_size = static_cast<int>(_tabsep - entry_word);
    // *_tabsep = 0;
    this->entry_word = _tabsep + 1;
    this->entry_word_size  = entry_word_size - this->entry_index_size - 1;
  } else {
    this->entry_word = this->entry_index = entry_word;
    this->entry_word_size = this->entry_index_size = entry_word_size;
  }
  this->entry_word_attrib   = entry_word_attrib;
  this->charcode            = charcode;
  // printf("[%s (%d), %d], %d\n",
  //        entry_word_utf8(), entry_word_attrib, entry_word_size, charcode);

  this->data                = data;
  this->data_size           = data_size;

  this->is_retained         = false;

  this->_entry_word_utf8    = NULL;
  this->_jword_utf8         = NULL;

  this->_ext_flags          = 0;  // not read yet
  this->_ext_start_pos      = NULL;
  this->_pron_utf8          = NULL;
  this->_example_utf8       = NULL;
}

PDICDatafield::~PDICDatafield() {
  if (is_retained) {
    free(static_cast<void*>(entry_word));
    free(static_cast<void*>(data));
  }
  if (_entry_word_utf8) free(static_cast<void*>(_entry_word_utf8));
  if (_jword_utf8) free(static_cast<void*>(_jword_utf8));
  if (_pron_utf8) free(static_cast<void*>(_pron_utf8));
  if (_example_utf8) free(static_cast<void*>(_example_utf8));

  if (_ext_flags) {
    traverse(_ext, data) {
      free(static_cast<void*>(data->second));
    }
    _ext.clear();
  }
}

void PDICDatafield::retain() {
  if (!is_retained) {
    entry_word = clone_cstr(entry_word, 0, false);
    data = clone_cstr(data, data_size, false);
    is_retained = true;
  }
}

int PDICDatafield::read_extension() {
  if (_ext_flags) return _ext_flags;

  if (!_ext_start_pos) {
    int jword_size = strlen(reinterpret_cast<char*>(data));
    _ext_start_pos = data + jword_size + 1;
  }

  _ext_flags = EXT_IS_READ;
  _ext.clear();

  for (byte *p = _ext_start_pos; p < data+data_size; ) {
    int ext_attrib = *p++;

    byte *ext_data = NULL;
    int ext_data_size = 0;

    if (ext_attrib & 0x80) {  // 拡張終了
      break;
    } else if (ext_attrib & 0x10) {  // バイナリデータ
      ext_data_size = u16val(p);
      p += 2;
      ext_data = p;
      p += ext_data_size;
    } else {
      ext_data = p;
      ext_data_size = strlen(reinterpret_cast<char*>(ext_data));
      p += ext_data_size + 1;
    }

    byte *ext_data_utf8 = NULL;
    switch (charcode) {
      case CHARCODE_BOCU1:
        ext_data_utf8 = bocu1_to_utf8(ext_data, ext_data_size);
        break;
      case CHARCODE_SHIFTJIS:
        ext_data_utf8 = sjis_to_utf8(ext_data, ext_data_size);
        break;
      default:
        ext_data_utf8 = clone_cstr(ext_data, ext_data_size, false);
        break;
    }

    switch (ext_attrib & 0x0f) {
      case EXT_EXAMPLE:  // 用例
        _ext[EXT_EXAMPLE] = ext_data_utf8;
        _ext_flags |= EXT_HAS_EXAMPLE;
        break;
      case EXT_PRON:  // 発音記号
        _ext[EXT_PRON] = ext_data_utf8;
        _ext_flags |= EXT_HAS_PRON;
        break;
      case EXT_RESERVED:  // 未定義
        break;
      case EXT_LINKDATA:  // リンクデータ
        _ext[EXT_LINKDATA] = ext_data_utf8;
        _ext_flags |= EXT_HAS_LINKDATA;
        break;
      default:
        break;
    }
  }
  return _ext_flags;
}

byte* PDICDatafield::in_utf8(int field) {
  switch (field) {
    case F_ENTRY:
      return entry_word_utf8();
    case F_JWORD:
      return jword_utf8();
    case F_EXAMPLE:
      return example_utf8();
    case F_PRON:
      return pron_utf8();
    default:
      return BYTE(NULL);
  }
}

byte* PDICDatafield::entry_word_utf8() {
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

byte* PDICDatafield::jword_utf8() {
  if (!_jword_utf8) {
    byte *jword = data;
    int jword_size = 0;
    if (entry_word_attrib & 0x10) {
      jword_size = strlen(reinterpret_cast<char*>(jword));
      // ofs += jword_datalength + 1;
      // printf("\n <%d: %d %d %02x>",
      //        field_id,  field_length, compress_length, entry_word_attrib);
      _ext_start_pos = jword + jword_size + 1;
    } else {
      jword_size = data_size;
      _ext_start_pos = BYTE(NULL);
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

byte* PDICDatafield::example_utf8() {  // 用例
  if (entry_word_attrib & 0x10) {
    if (read_extension() & EXT_HAS_EXAMPLE) {
      return _ext[EXT_EXAMPLE];
    }
  }
  return BYTE(NULL);
}

byte* PDICDatafield::pron_utf8() {  // 発音記号
  if (entry_word_attrib & 0x10) {
    if (read_extension() & EXT_HAS_PRON) {
      return _ext[EXT_PRON];
    }
  }
  return BYTE(NULL);
}


