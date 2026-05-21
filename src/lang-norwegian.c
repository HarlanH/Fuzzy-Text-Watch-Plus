#include "num2words.h"

// Norska, bokmål
// Translation by Øystein Raabe

const Language LANG_NORWEGIAN = {

  .hours = {
    "ett",
    "to",
    "tre",
    "fire",
    "fem",
    "seks",
    "sju",
    "åtte",
    "ni",
    "ti",
    "elleve",
    "tolv"
  },

#ifdef SCREEN_WIDE
  .phrases = {
    "*$1 ",
    "fem over *$1 ",
    "ti over *$1 ",
    "kvart over *$1 ",
    "ti på halv *$2 ",
    "fem på halv *$2 ",
    "halv *$2 ",
    "fem  over halv *$2 ",
    "ti over halv *$2 ",
    "kvart på *$2 ",
    "ti på *$2 ",
    "fem på *$2 "
  },

  .greetings = {
    "God morgen ",
    "God dag ",
    "God  kveld ",
    "God natt "
  },

  .connection_lost = "Hvor  er tele- fonen din? ",
#else
  .phrases = {
    "*$1 ",
    "fem  over *$1 ",
    "ti over *$1 ",
    "kvart over *$1 ",
    "ti på halv *$2 ",
    "fem på halv *$2 ",
    "halv *$2 ",
    "fem  over  halv *$2 ",
    "ti over halv *$2 ",
    "kvart  på *$2 ",
    "ti på *$2 ",
    "fem på *$2 "
  },

  .greetings = {
    "God  mor-  gen ",
    "God  dag ",
    "God  kveld ",
    "God  natt "
  },

  .connection_lost = "Hvor er tele- fonen din? ",

#endif
  .meeting_now = "Mote Naa!",
  .meeting_soon = "Mote Snart!",


  .number_of_exceptions = 2,
  .exceptions = {
    {
      .hours = 0,
      .pentaminutes = 0,
      .phrase = "*midnatt "
    },
    {
      .hours = 12,
      .pentaminutes = 0,
      .phrase = "*middag "
    }
  }
};
