#include "num2words.h"

// Translation by David Ortmann

const Language LANG_GERMAN_W = {
  .hours = {
    "eins",
    "zwei",
    "drei",
    "vier",
    "fünf",
    "sechs",
    "sieben",
    "acht",
    "neun",
    "zehn",
    "elf",
    "zwölf"
  },

#ifdef SCREEN_WIDE
  .phrases = {
    "*$1 uhr ",
    "fünf nach *$1 ",
    "zehn  nach *$1 ",
    "viertel nach *$1 ",
    "zwanzig nach *$1 ",
    "fünf vor halb *$2 ",
    "halb *$2 ",
    "fünf  nach halb *$2 ",
    "zwanzig vor *$2 ",
    "viertel vor *$2 ",
    "zehn vor *$2 ",
    "fünf vor *$2 "
  },

  .greetings = {
    "Guten Morgen ",
    "Guten  Tag ",
    "Guten Abend ",
    "Gute Nacht "
  },
#else
  .phrases = {
    "*$1 uhr ",
    "fünf  nach *$1 ",
    "zehn  nach *$1 ",
    "viertel nach *$1 ",
    "zwan-  zig  nach *$1 ",
    "fünf vor halb *$2 ",
    "halb *$2 ",
    "fünf  nach  halb *$2 ",
    "zwan-  zig vor *$2 ",
    "viertel vor *$2 ",
    "zehn  vor *$2 ",
    "fünf  vor *$2 "
  },

  .greetings = {
    "Guten Morgen ",
    "Guten  Tag ",
    "Guten Abend ",
    "Gute Nacht "
  },
#endif

  .connection_lost = "Wo ist dein Handy? ",
  .meeting_now = "Treffen jetzt!",
  .meeting_soon = "Treffen bald!",

  .number_of_exceptions = 4,

  .exceptions = {
     {
       .hours = 0,
       .pentaminutes = 0,
       .phrase = "*Mittern- acht "
     },
     {
       .hours = 12,
       .pentaminutes = 0,
       .phrase = "*Mittag "
     },
     {
       .hours = 1,
       .pentaminutes = 0,
       .phrase = "*ein uhr "
     },
     {
       .hours = 13,
       .pentaminutes = 0,
       .phrase = "*ein uhr "
     }
  }
};
