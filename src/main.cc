#include <iostream>
#include <string>
#include <cstdio>

#include "shell.h"
#include "dump.h"

extern std::string current_dict_name;

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
        looping = do_command(line+1);
        break;

      case '!': // external shell mode
        system(line+1);
        break;

      default:
        do_lookup(line);
        break;
    }
  }

  shell_destroy();
  
  return 0;
}
