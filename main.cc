// Copyright 2012 naoya_t.  All Rights Reserved.
// Use of this source code is governed by a LGPL-style
// license that can be found in the COPYING file.

#include <stdio.h>

#include <string>

#include "pdic/lookup.h"
#include "util/ansi_color.h"
#include "util/dump.h"
#include "util/Shell.h"

Shell *g_shell = NULL;

int main(int argc, char **argv) {
  printf(ANSI_UNDERLINE_ON
         "PDIC CUI Search ver 0.7 (c)2012 @naoya_t. All Rights Reserved."
         ANSI_UNDERLINE_OFF "\n");

  g_shell = new Shell();

  printf("読み込み中...\n");
  g_shell->load_rc();

  // if (argc >= 2) {
  //   std::string filename = argv[1];
  //   g_shell->do_load(filename);
  // }

  // REPL
  for (bool looping = true; looping; ) {
    printf("%s> ", g_shell->current_dict_name.c_str());
    char line[256];
    if (!fgets(line, 256, stdin)) {
      printf("\n");
      break;
    }
    int linelen = strlen(line);
    line[--linelen] = 0;
    if (linelen == 0) continue;

    switch (line[0]) {
      case '?':  // reserved (help)
        break;

      case '+':
        full_lookup(reinterpret_cast<byte*>(line)+1, linelen-1);
        break;

      case '.':  // command mode
        if (linelen > 1) {
          looping = g_shell->do_command(line+1);
        }
        break;

      case '!':  // external shell mode
        if (linelen > 1) {
          system(line+1);
        }
        break;

      case '*':
        if (linelen > 1) {
          sarray_lookup(reinterpret_cast<byte*>(line)+1, linelen-1);
        }
        break;

      case '/':
        if (linelen >= 3) {
          if (strchr(line+1, '/') == line + linelen - 1) {
            line[linelen-1] = 0;
            regexp_lookup(reinterpret_cast<byte*>(line)+1, linelen-2);
            break;
          }
        }
        // else fall thru

      default:
        default_lookup(reinterpret_cast<byte*>(line), linelen);
        break;
    }
  }

  delete g_shell;
  g_shell = NULL;

  printf("bye!\n");

  return 0;
}
