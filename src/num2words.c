#include "num2words.h"
#include "generated/ui_i18n_gen.h"
#include "string.h"
#include <stdio.h>

#include "lang-swedish.h"
#include "lang-english.h"
#include "lang-english-uk.h"
#include "lang-norwegian.h"
#include "lang-dutch.h"
#include "lang-italian.h"
#include "lang-spanish.h"
#include "lang-german-eastern.h"
#include "lang-german-western.h"
#include "lang-french.h"
#include "lang-japanese.h"

static const Language* language = &LANG_ENGLISH;
static uint8_t current_language_id = LANG_EN;

void set_language(uint8_t lang) {
  current_language_id = lang;
  switch (lang) {
    case LANG_EN:
      language = &LANG_ENGLISH;
      break;

    case LANG_EN_GB:
      language = &LANG_ENGLISH_UK;
      break;

    case LANG_SE:
      language = &LANG_SWEDISH;
      break;

    case LANG_NO:
      language = &LANG_NORWEGIAN;
      break;

    case LANG_NL:
      language = &LANG_DUTCH;
      break;

    case LANG_IT:
      language = &LANG_ITALIAN;
      break;

    case LANG_ES:
      language = &LANG_SPANISH;
      break;

    case LANG_GE:
      language = &LANG_GERMAN_E;
      break;

    case LANG_GW:
      language = &LANG_GERMAN_W;
      break;

    case LANG_FR:
      language = &LANG_FRENCH;
      break;

    case LANG_JA:
      language = &LANG_JAPANESE;
      break;

    default:
      current_language_id = LANG_EN;
      language = &LANG_ENGLISH;
  }
}

uint8_t get_current_language_id(void) {
  return current_language_id;
}

const char* getHourWord(int hour) {
  int pos = (hour + 11) % 12; // + 11 instead of - 1 to avoid negative result
  return language->hours[pos];
}

const char* getFiveMinutePhrase(int fiveMinutePeriod) {
  return language->phrases[fiveMinutePeriod];
}


void check_exceptions(int hours, int pentaminutes, char* words, size_t length) {
  int i;
  for (i = 0; i < language->number_of_exceptions; i++) {
    if (hours == language->exceptions[i].hours) {
      if (pentaminutes == language->exceptions[i].pentaminutes) {
        strncpy(words, language->exceptions[i].phrase, length);
        return;
      }
    }
  }
}

// First five-minute bucket of the hour ("… o'clock")
static bool raw_in_oclock_window(struct tm *raw, int hour_adj) {
  return raw->tm_hour == hour_adj && raw->tm_min < 5;
}

// Bucket for "… thirty" (half past)
static bool raw_in_half_window(struct tm *raw, int hour_adj) {
  return raw->tm_hour == hour_adj && raw->tm_min >= 30 && raw->tm_min < 35;
}

// Stitch prefix + hour + suffix; do not use phrase as a printf format (literal * must survive).
static void substitute_hour_in_phrase(char *out, size_t out_len, const char *phrase,
                                      const char *hour, const char *var) {
  int prefix_len = (int)(var - phrase);
  const char *suffix = var + 2;
  snprintf(out, out_len, "%.*s%s%s", prefix_len, phrase, hour, suffix);
}

static void time_to_words_impl(int hours, int minutes, char* words, size_t length) {
  memset(words, 0, length);

  int fiveMinutePeriod = (minutes / 5) % 12;

  check_exceptions(hours, fiveMinutePeriod, words, length);

  if (*words == '\0') {
    char phrase[length];
    strcpy(phrase, getFiveMinutePhrase(fiveMinutePeriod));

    char *variable = NULL;
    const char *hour;

    if ((variable = strstr(phrase, "$1")) != NULL) {
      hour = getHourWord(hours);
    }
    else if ((variable = strstr(phrase, "$2")) != NULL) {
      hour = getHourWord(hours + 1);
    }

    if (variable != NULL) {
      substitute_hour_in_phrase(words, length, phrase, hour, variable);
    } else {
      strncpy(words, phrase, length);
    }
  }
}

void time_to_words(int hours, int minutes, char* words, size_t length,
                   bool strict_hour_phrases, struct tm *raw_local) {
  int fiveMinutePeriod = (minutes / 5) % 12;

  if (strict_hour_phrases && raw_local != NULL) {
    if (fiveMinutePeriod == 0 && !raw_in_oclock_window(raw_local, hours)) {
      int h = hours - 1;
      if (h < 0) {
        h += 24;
      }
      time_to_words_impl(h, 55, words, length);
      return;
    }
    if (fiveMinutePeriod == 6 && !raw_in_half_window(raw_local, hours)) {
      time_to_words_impl(hours, 25, words, length);
      return;
    }
  }

  time_to_words_impl(hours, minutes, words, length);
}

void time_to_greeting(int hour, char* greeting)
{
  int pos;

  if (hour < 6) {
    pos = 3; // night greeting
  } else if (hour < 12) {
    pos = 0; // morning greeting
  } else if (hour < 18) {
    pos = 1; // day greeting
  } else {
    pos = 2; // evening greeting
  }

  strcpy(greeting, language->greetings[pos]);
}

void get_connection_lost_message(char* message)
{
  strcpy(message, language->connection_lost);
}

void get_day_of_month_message(int day, char* message, size_t length)
{
  if (day < 1 || day > 31) {
    message[0] = '\0';
    return;
  }

  ui_format_day_of_month(get_current_language_id(), day, message, length);
}

void get_meeting_now_message(char* message, size_t length)
{
  snprintf(message, length, "%s", language->meeting_now);
}

void get_meeting_soon_message(char* message, size_t length)
{
  snprintf(message, length, "%s", language->meeting_soon);
}

const char* get_battery_low_label(void)
{
  return ui_bat_low_for_lang(get_current_language_id());
}
