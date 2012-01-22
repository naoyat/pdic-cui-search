#include <iostream>
#include <string>
#include <cstdio>

#include "shell.h"
#include "lookup.h"
#include "dump.h"
#include "ansi_color.h"

extern std::string current_dict_name;
extern bool verbose_mode;

int main(int argc, char **argv)
{
  std::cout << ANSI_UNDERLINE_ON "PDIC CUI Search ver 0.7 (c)2012 @naoya_t. All Rights Reserved." ANSI_UNDERLINE_OFF << std::endl;
  std::cout << "読み込み中..." << std::endl;
  shell_init();

  if (argc >= 2) {
    std::string filename = argv[1];
    do_load(filename);
  }

  // REPL
  for (bool looping=true; looping; ) {
    std::cout << current_dict_name << "> ";
    char line[256];
    if (!fgets(line, 256, stdin)) { newline(); break; }
    int linelen = strlen(line); line[--linelen] = 0;
    if (linelen == 0) continue;

    switch (line[0]) {
      case '?': // reserved (help)
        break;

      case '+':
        full_lookup((byte*)line+1, linelen-1);
        break;

      case '.': // command mode
        if (linelen > 1) {
          if (verbose_mode) {
            //printf("[COMMAND] %s\n", line+1);
          }
          looping = do_command(line+1);
        }
        break;

      case '!': // external shell mode
        if (linelen > 1) {
          if (verbose_mode) {
            //printf("[EXTERNAL] %s\n", line+1);
          }
          system(line+1);
        }
        break;

      case '*':
        if (linelen > 1) {
          if (verbose_mode) {
            //printf("[LOOKUP<sarray>] %s\n", line+1);
          }
          sarray_lookup((byte*)line+1, linelen-1);
        }
        break;

      case '/':
        if (linelen >= 3) {
          if (strchr(line+1,'/') == line + linelen - 1)  {
            line[linelen-1] = 0;
            if (verbose_mode) {
              //printf("[LOOKUP<regexp>] /%s/\n", line+1);
            }
            regexp_lookup((byte*)line+1, linelen-2);
            break;
          }
        }
        // else fall thru

      default:
        if (verbose_mode) {
          //printf("[LOOKUP<normal>] %s\n", line);
        }
        default_lookup((byte*)line, linelen);
        break;
    }
  }

  shell_destroy();

  std::cout << "bye!" << std::endl;
  return 0;
}
