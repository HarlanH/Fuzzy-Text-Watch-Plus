#include "num2words.h"

const Language LANG_JAPANESE = {
  .hours = {
    "ichi",
    "ni",
    "san",
    "shi",
    "go",
    "roku",
    "shichi",
    "hachi",
    "ku",
    "ju",
    "juuichi",
    "juuni"
  },

  .phrases = {
    "*$1 ji ",
    "*$1 ji gofun ",
    "*$1 ji juppun ",
    "*$1 ji juugofun ",
    "*$1 ji nijuppun ",
    "*$1 ji nijuugofun ",
    "*$1 ji han ",
    "*$2 ji nijuugofun mae ",
    "*$2 ji nijuppun mae ",
    "*$2 ji juugofun mae ",
    "*$2 ji juppun mae ",
    "*$2 ji gofun mae "
  },

  .greetings = {
    "Ohayo ",
    "Konnichiwa ",
    "Konbanwa ",
    "Oyasumi "
  },
  .connection_lost = "Denwa wa doko? ",
  .meeting_now = "Kaigi ima!",
  .meeting_soon = "Kaigi sugu!",

  .number_of_exceptions = 2,
  .exceptions = {
    {
      .hours = 0,
      .pentaminutes = 0,
      .phrase = "*shinya "
    },
    {
      .hours = 12,
      .pentaminutes = 0,
      .phrase = "*shougo "
    }
  }
};
