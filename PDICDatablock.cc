#include "PDICDatablock.h"
#include "PDICHeader.h"
#include "PDICDatafield.h"

#include "util.h"
#include "bocu1.h"
#include "charcode.h"

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
  unsigned char entry_word[1024]; // （圧縮見出し語の伸長用）見出し語バッファ。Ver6でlword=1024なの
  
  //  unsigned char *top_word;
  //  int top_word_length = 0;

  for (int ofs=0,field_id=0; ofs<_datablock_buf_size; ++field_id) {
    bool matched = false;

    // +0
    int field_length, compress_length, entry_word_attrib, next_ofs;
    unsigned char *entry_word_compressed;
    int entry_word_compressed_size;

    if (_is4byte) {
      field_length = longval(_datablock_buf + ofs); ofs += 4;
    } else {
      field_length = shortval(_datablock_buf + ofs); ofs += 2;
      if (field_length == 0) break;
    }

    if (_isAligned) {
      compress_length = _datablock_buf[ofs++];
      entry_word_attrib = _datablock_buf[ofs++];
      next_ofs = ofs + field_length;

      entry_word_compressed = _datablock_buf + ofs;
      entry_word_compressed_size = strlen((char *)entry_word_compressed);
      ofs += entry_word_compressed_size + 1;
    } else {
      compress_length = _datablock_buf[ofs++];
      next_ofs = ofs + field_length;

      entry_word_compressed = _datablock_buf + ofs;
      entry_word_compressed_size = strlen((char *)entry_word_compressed);
      ofs += entry_word_compressed_size + 1;

      entry_word_attrib = _datablock_buf[ofs++];
    }

    // uchar* entry_word, int entry_word_attrib, uchar* data, int datasize
    memcpy(entry_word + compress_length, entry_word_compressed, entry_word_compressed_size+1);

    PDICDatafield datafield(entry_word,
                            compress_length + entry_word_compressed_size, // entry_word_size
                            entry_word_attrib,
                            _index->isBOCU1() ? CHARCODE_BOCU1 : CHARCODE_SHIFTJIS,
                            _datablock_buf + ofs, // data
                            next_ofs - ofs // datasize
                            );

    /*
    unsigned char *tabsep_at = (unsigned char *)strchr((char *)word_uncompress_buf, '\t');
    unsigned char *word_body;
    int entry_len;
    if (tabsep_at) {
      word_body = tabsep_at + 1;
      entry_len = tabsep_at - word_buf;
    } else {
      word_body = word_buf;
      entry_len = compress_length + entry_word_datalength;
    }
    */
    if (criteria) {
      matched = criteria->match(&datafield);
    } else {
      matched = true;
    }

    if (matched) {
      (*action)(&datafield);
      /*
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
      (*action)(word_buf, _utf8, jword_utf8);
      
      free((void *)entry_utf8);
      free((void *)jword_utf8);
      */
    }
    
    if (_is4byte) break;
    ofs = next_ofs;
  }
}
