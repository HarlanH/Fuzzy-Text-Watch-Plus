#include "num2words.h"

const Language LANG_SPANISH = {

  .hours = {
      "una",
      "dos",
      "tres",
      "cuatro",
      "cinco",
      "seis",
      "siete",
      "ocho",
      "nueve",
      "diez",
      "once",
      "doce"
  },

#ifdef SCREEN_WIDE
  .phrases = {
      "*$1 en punto ",
      "*$1 y cinco ",
      "*$1 y diez ",
      "*$1 y cuarto ",
      "*$1 y veinte ",
      "*$1 y veinti- cinco ",
      "*$1 y media ",
      "*$2 menos veinti- cinco ",
      "*$2 menos veinte ",
      "*$2 menos cuarto ",
      "*$2 menos diez ",
      "*$2 menos cinco "
  },
#else
  .phrases = {
      "*$1 en  punto ",
      "*$1 y cinco ",
      "*$1 y diez ",
      "*$1 y  cuarto ",
      "*$1 y veinte ",
      "*$1 y veinti- cinco ",
      "*$1 y media ",
      "*$2 menos veinti- cinco ",
      "*$2 menos veinte ",
      "*$2 menos cuarto ",
      "*$2 menos diez ",
      "*$2 menos cinco "
  },
#endif

  .greetings = {
    "Buenos días ",
    "Buenos días ",
    "Buenas tardes ",
    "Buenas noches "
  },

#ifdef SCREEN_WIDE
  .connection_lost = "Dónde está tu teléfono ",
#else
  .connection_lost = "Dónde está tu telé- fono ",
#endif
  .meeting_now = "Reunión Ya!",
  .meeting_soon = "Reunión Pronto!",


  .number_of_exceptions = 2,
  .exceptions = {
    {
      .hours = 0,
      .pentaminutes = 0,
      .phrase = "*media- noche "
    },
    {
      .hours = 12,
      .pentaminutes = 0,
      .phrase = "*medio- día "
    }
  }
};
