#include "num2words.h"

/* Informal American English (default LANG_EN) */

const Language LANG_ENGLISH = {
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

  .phrases = {
    "*$1 o'clock ",
    "*$1 o'five ",
    "*$1 ten ",
    "*$1 fifteen ",
    "*$1 twenty ",
    "*$1 twenty five ",
    "*$1 thirty ",
    "*$1 thirty five ",
    "twenty til *$2 ",
    "quarter til *$2 ",
    "ten til *$2 ",
    "almost *$2 "
  },

#ifdef SCREEN_WIDE
  .greetings = {
    "Good morning ",
    "Good afternoon ",
    "Good evening ",
    "Good night "
  },
#else
  .greetings = {
    "Good  mor-  ning ",
    "Good after- noon ",
    "Good even-  ing ",
    "Good night "
  },
#endif

  .connection_lost = "Where's your phone? ",
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
      .phrase = "*noon "
    }
  }
};
