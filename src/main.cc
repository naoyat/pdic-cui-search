#include <iostream>
#include <string>
#include <cstdio>

#include "shell.h"
#include "dump.h"

extern std::string current_dict_name;
extern bool verbose_mode;

int main(int argc, char **argv)
{
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
          do_sarray_lookup(line+1, linelen-1);
        }
        break;

      case '/':
        if (linelen >= 3) {
          if (strchr(line+1,'/') == line + linelen - 1)  {
            line[linelen-1] = 0;
            if (verbose_mode) {
              //printf("[LOOKUP<regexp>] /%s/\n", line+1);
            }
            do_regexp_lookup(line+1, linelen-2);
            break;
          }
        }
        // else fall thru

      default:
        if (verbose_mode) {
          //printf("[LOOKUP<normal>] %s\n", line);
        }
        do_normal_lookup(line, linelen);
        break;
    }
  }

  shell_destroy();

  printf("bye.\n");
  return 0;
}
