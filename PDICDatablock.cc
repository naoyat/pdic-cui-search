#include "PDICDatablock.h"
#include "PDICHeader.h"
#include "util.h"
#include "bocu1.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>


PDICDatablock::PDICDatablock(FILE *fp, PDICIndex *index, int ix)
{
  _index = index;
  unsigned int datablock_offset = index->datablock_offset(ix);
  unsigned int block_size = index->datablock_block_size();
  _isAligned = index->header->isAligned();

  fseek(fp, datablock_offset, SEEK_SET);
  unsigned char blocks_buf[2];
  size_t size = fread(blocks_buf, 2, 1, fp);
  int using_blocks_count = ushortval(blocks_buf);
  _is4byte = using_blocks_count & 0x8000; using_blocks_count &= 0x7fff;

  _datablock_buf_size = block_size * using_blocks_count - 2;
  //printf("使用ブロック数: %d (%d); %d bytes\n", using_blocks_count, is4byte, datablock_buf_size);

  _datablock_buf = new unsigned char[_datablock_buf_size];
  if (_datablock_buf == NULL) return;

  size = fread(_datablock_buf, _datablock_buf_size, 1, fp);
  if (size != 1) return;
}
PDICDatablock::~PDICDatablock()
{
  if (_datablock_buf) delete _datablock_buf;
}

void
PDICDatablock::iterate(action_proc *action, Criteria *criteria)
{
  unsigned char word_buf[1024]; // （圧縮見出し語の伸長用）見出し語バッファ。Ver6でlword=1024なの
  
  unsigned char *top_word;
  int top_word_length = 0;

  for (int ofs=0,field_id=0; ofs<_datablock_buf_size; ++field_id) {
    bool matched = false;

    // +0
    int field_length, compress_length, entry_word_attrib, next_ofs;
    unsigned char *entry_word;
    int entry_word_datalength;

    if (_is4byte) {
      field_length = longval(_datablock_buf + ofs); ofs += 4;
    } else {
      field_length = shortval(_datablock_buf + ofs); ofs += 2;
      if (field_length == 0) break;
    }

    if (_isAligned) {
      compress_length = ubyteval(_datablock_buf + ofs++);
      entry_word_attrib = ubyteval(_datablock_buf + ofs++);
      next_ofs = ofs + field_length;

      entry_word = _datablock_buf + ofs;
      entry_word_datalength = strlen((char *)entry_word);
      ofs += entry_word_datalength + 1;
    } else {
      compress_length = ubyteval(_datablock_buf + ofs++);
      next_ofs = ofs + field_length;

      entry_word = _datablock_buf + ofs;
      entry_word_datalength = strlen((char *)entry_word);
      ofs += entry_word_datalength + 1;

      entry_word_attrib = ubyteval(_datablock_buf + ofs++);
    }

    if (field_id == 0) {
      top_word = entry_word; // データブロックの最初の単語(の先頭位置)を保持
      top_word_length = entry_word_datalength;
      
      memcpy(word_buf, entry_word, entry_word_datalength+1);
    }
    else {
      //if (compress_length > 0) memcpy(word_buf, top_word, top_word_length+1);
      memcpy(word_buf + compress_length, entry_word, entry_word_datalength+1);
    }

    unsigned char *tabsep_at = (unsigned char *)strchr((char *)word_buf, '\t');
    unsigned char *word_body;
    int entry_len;
    if (tabsep_at) {
      word_body = tabsep_at + 1;
      entry_len = tabsep_at - word_buf;
    } else {
      word_body = word_buf;
      entry_len = compress_length + entry_word_datalength;
    }

    if (criteria) {
      matched = criteria->match(word_buf, entry_len);
    } else {
      matched = true;
    }

    unsigned char *jword = _datablock_buf + ofs;
    int jword_datalength = 0;
    if (entry_word_attrib & 0x10) {
      jword_datalength = strlen((char *)jword);
      //printf("\n <%d: %d %d %02x>", field_id,  field_length, compress_length, entry_word_attrib);
      ofs += jword_datalength + 1;
    } else {
      jword_datalength = next_ofs - ofs;
    }

    if (matched) {
      unsigned char *entry_utf8, *jword_utf8;
      if (_index->isBOCU1()) {
        entry_utf8 = bocu1_to_utf8(word_body);
        jword_utf8 = bocu1_to_utf8(jword, jword_datalength);
      } else {
        //unsigned char *jword0 = (unsigned char *)cstr(jword, jword_datalength);
        entry_utf8 = sjis_to_utf8(word_body);
        jword_utf8 = sjis_to_utf8(jword, jword_datalength);
      }
      
      (*action)(entry_utf8, jword_utf8);
      
      free((void *)entry_utf8);
      free((void *)jword_utf8);
    }
    
    if (_is4byte) break;
    ofs = next_ofs;
  }
}
