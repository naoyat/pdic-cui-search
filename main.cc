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
         "PDIC CUI Search ver 0.7.1 (c)2012 @naoya_t. All Rights Reserved."
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

    int head = line[0], tail = line[linelen-1];
    switch (head) {
      case '?':  // reserved (help)
        break;

      case '+':
        lookup(reinterpret_cast<byte*>(line)+1, LOOKUP_FROM_ALL);
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
          lookup(reinterpret_cast<byte*>(line)+1, LOOKUP_SARRAY);
        }
        break;

      case '/':
        if (linelen >= 3 && tail == '/') {
          line[linelen-1] = '\0';
          lookup(reinterpret_cast<byte*>(line)+1, LOOKUP_REGEXP);
          break;
        }
        // else fall thru

      default:
        int flags = g_shell->params.default_lookup_flags;
        if (tail == '*') {
          line[linelen-1] = '\0';
          flags &= ~LOOKUP_MATCH_BACKWARD;
        }
        lookup(reinterpret_cast<byte*>(line), flags);
        break;
    }
  }

  delete g_shell;
  g_shell = NULL;

  printf("bye!\n");

  return 0;
}
