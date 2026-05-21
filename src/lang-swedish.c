#include "num2words.h"

const Language LANG_SWEDISH = {
  .hours = {
    "ett",
    "två",
    "tre",
    "fyra",
    "fem",
    "sex",
    "sju",
    "åtta",
    "nio",
    "tio",
    "elva",
    "tolv"
  },

#ifdef SCREEN_WIDE
  .phrases = {
    "klockan är *$1 ",
    "fem över *$1 ",
    "tio över *$1 ",
    "kvart över *$1 ",
    "tjugo över *$1 ",
    "fem i halv *$2 ",
    "halv *$2 ",
    "fem  över halv *$2 ",
    "tjugo i *$2 ",
    "kvart i *$2 ",
    "tio i *$2 ",
    "fem i *$2 "
  },

  .greetings = {
    "God morgon ",
    "God dag ",
    "God  afton ",
    "God natt "
  },
#else
  .phrases = {
    "klockan är *$1 ",
    "fem  över *$1 ",
    "tio över *$1 ",
    "kvart över *$1 ",
    "tjugo över *$1 ",
    "fem i halv *$2 ",
    "halv *$2 ",
    "fem  över  halv *$2 ",
    "tjugo i *$2 ",
    "kvart i *$2 ",
    "tio i *$2 ",
    "fem i *$2 "
  },

  .greetings = {
    "God  mor-  gon ",
    "God  dag ",
    "God  afton ",
    "God  natt "
  },
#endif

  .connection_lost = "Var är din telefon ",
  .meeting_now = "Mote Nu!",
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
