#ifndef PDICCUISEARCH_ANSICOLOR_H_
#define PDICCUISEARCH_ANSICOLOR_H_

// reset colors & styles
#define ANSI_RESET             "\x1b[0m"

// set style
#define ANSI_BOLD_ON           "\x1b[1m"
#define ANSI_ITALICS_ON        "\x1b[3m"
#define ANSI_UNDERLINE_ON      "\x1b[4m"
#define ANSI_INVERSE_ON        "\x1b[7m" // reverses fgcolor/bgcolor
#define ANSI_STRIKETHROUGH_ON  "\x1b[9m"

// unset style
#define ANSI_BOLD_OFF          "\x1b[22m"
#define ANSI_ITALICS_OFF       "\x1b[23m"
#define ANSI_UNDERLINE_OFF     "\x1b[24m"
#define ANSI_INVERSE_OFF       "\x1b[27m"
#define ANSI_STRIKETHROUGH_OFF "\x1b[29m"

// set foreground color
#define ANSI_FGCOLOR_BLACK     "\x1b[30m"
#define ANSI_FGCOLOR_RED       "\x1b[31m"
#define ANSI_FGCOLOR_GREEN     "\x1b[32m"
#define ANSI_FGCOLOR_YELLOW    "\x1b[33m"
#define ANSI_FGCOLOR_BLUE      "\x1b[34m"
#define ANSI_FGCOLOR_MAGENTA   "\x1b[35m" // purple
#define ANSI_FGCOLOR_CYAN      "\x1b[36m"
#define ANSI_FGCOLOR_WHITE     "\x1b[37m"
#define ANSI_FGCOLOR_DEFAULT   "\x1b[39m"

// set background color
#define ANSI_BGCOLOR_BLACK     "\x1b[40m"
#define ANSI_BGCOLOR_RED       "\x1b[41m"
#define ANSI_BGCOLOR_GREEN     "\x1b[42m"
#define ANSI_BGCOLOR_YELLOW    "\x1b[43m"
#define ANSI_BGCOLOR_BLUE      "\x1b[44m"
#define ANSI_BGCOLOR_MAGENTA   "\x1b[45m"
#define ANSI_BGCOLOR_CYAN      "\x1b[46m"
#define ANSI_BGCOLOR_WHITE     "\x1b[47m"
#define ANSI_BGCOLOR_DEFAULT   "\x1b[49m"

#endif // PDICCUISEARCH_ANSICOLOR_H_
