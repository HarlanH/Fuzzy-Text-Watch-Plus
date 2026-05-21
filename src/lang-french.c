#include "num2words.h"

const Language LANG_FRENCH = {
  .hours = {
    "une",
    "deux",
    "trois",
    "quatre",
    "cinq",
    "six",
    "sept",
    "huit",
    "neuf",
    "dix",
    "onze",
    "douze"
  },

#ifdef SCREEN_WIDE
  .phrases = {
    "*$1 heure ",
    "*$1 et cinq ",
    "*$1 et dix ",
    "*$1 et quart ",
    "*$1 et vingt ",
    "*$1 et vingt- cinq ",
    "*$1 et demie ",
    "*$2 moins vingt- cinq ",
    "*$2 moins vingt ",
    "*$2 moins le quart ",
    "*$2 moins dix ",
    "*$2 moins cinq "
  },
#else
  .phrases = {
    "*$1 heure ",
    "*$1 et cinq ",
    "*$1 et dix ",
    "*$1 et quart ",
    "*$1 et vingt ",
    "*$1 et vingt- cinq ",
    "*$1 et demie ",
    "*$2 moins vingt- cinq ",
    "*$2 moins vingt ",
    "*$2 moins le quart ",
    "*$2 moins dix ",
    "*$2 moins cinq "
  },
#endif

  .greetings = {
    "Bonjour ",
    "Bon après- midi ",
    "Bonsoir ",
    "Bonne nuit "
  },
  .connection_lost = "Où est ton télé- phone? ",
  .meeting_now = "Réunion !",
  .meeting_soon = "Réunion Bientôt !",

  .number_of_exceptions = 4,
  .exceptions = {
    {
      .hours = 0,
      .pentaminutes = 0,
      .phrase = "*minuit "
    },
    {
      .hours = 12,
      .pentaminutes = 0,
      .phrase = "*midi "
    },
    {
      .hours = 1,
      .pentaminutes = 0,
      .phrase = "*une heure "
    },
    {
      .hours = 13,
      .pentaminutes = 0,
      .phrase = "*une heure "
    }
  }
};
