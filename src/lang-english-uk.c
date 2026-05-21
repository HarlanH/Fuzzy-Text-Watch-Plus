#include "num2words.h"

const Language LANG_ENGLISH_UK = {
  .hours = {
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven",
    "twelve"
  },

#ifdef SCREEN_WIDE
  .phrases = {
    "*$1 o'clock ",
    "five past *$1 ",
    "ten past *$1 ",
    "quarter past *$1 ",
    "twenty past *$1 ",
    "twenty five past *$1 ",
    "half past *$1 ",
    "twenty five to *$2 ",
    "twenty  to *$2 ",
    "quarter to *$2 ",
    "ten to *$2 ",
    "five to *$2 "
  },

  .greetings = {
    "Good morning ",
    "Good  day ",
    "Good evening ",
    "Good night "
  },
#else
  .phrases = {
    "*$1 o'clock ",
    "five  past *$1 ",
    "ten  past *$1 ",
    "quarter past *$1 ",
    "twenty past *$1 ",
    "twenty five  past *$1 ",
    "half  past *$1 ",
    "twenty five to *$2 ",
    "twenty  to *$2 ",
    "quarter to *$2 ",
    "ten to *$2 ",
    "five to *$2 "
  },

  .greetings = {
    "Good  mor-  ning ",
    "Good  day ",
    "Good even-  ing ",
    "Good night "
  },
#endif

  .connection_lost = "Where  is your phone? ",
  .meeting_now = "Meeting Now!",
  .meeting_soon = "Meeting Soon!",

  .number_of_exceptions = 2,
  .exceptions = {
    {
      .hours = 0,
      .pentaminutes = 0,
      .phrase = "*midnight "
    },
    {
      .hours = 12,
      .pentaminutes = 0,
      .phrase = "*midday "
    }
  }
};
