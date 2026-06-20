#include "config_message.h"
#include "TextWatch.h"

static void handle_shared_settings(DictionaryIterator *iter, const ConfigMessageContext *ctx) {
  Tuple *language_t = dict_find(iter, KEY_LANGUAGE);
  if (language_t) {
    ctx->set_language(language_t->value->uint8);
    persist_write_int(KEY_LANGUAGE, language_t->value->uint8);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Language is %d", language_t->value->uint8);
  }

  Tuple *offset_t = dict_find(iter, KEY_OFFSET);
  if (offset_t) {
    ctx->set_offset(offset_t->value->uint16);
    persist_write_int(KEY_OFFSET, offset_t->value->uint16);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Offset is %d", offset_t->value->uint16);
  }

  Tuple *gesture_t = dict_find(iter, KEY_GESTURE);
  if (gesture_t) {
    ctx->set_gesture(gesture_t->value->uint8);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Gesture is %d", gesture_t->value->uint8);
  }

  Tuple *bt_notification_t = dict_find(iter, KEY_BT_NOTIFICATION);
  if (bt_notification_t) {
    ctx->set_bt_lost_notification(bt_notification_t->value->uint8);
    persist_write_int(KEY_BT_NOTIFICATION, bt_notification_t->value->uint8);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "BT notification is %d", bt_notification_t->value->uint8);
  }

  Tuple *strict_hour_t = dict_find(iter, KEY_STRICT_HOUR_PHRASES);
  if (strict_hour_t) {
    bool strict = strict_hour_t->value->uint8 != 0;
    ctx->set_strict_hour_phrases(strict);
    persist_write_bool(KEY_STRICT_HOUR_PHRASES, strict);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Strict hour phrases is %d", strict ? 1 : 0);
  }
}

void config_message_handle_inbox(DictionaryIterator *iter, const ConfigMessageContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received inbox message");
  handle_shared_settings(iter, ctx);

#ifdef PBL_COLOR
  Tuple *background_color_t = dict_find(iter, KEY_BACKGROUND);
  if (background_color_t) {
    GColor8 bg_color;
    bg_color.argb = background_color_t->value->uint8;
    window_set_background_color(ctx->window, bg_color);
    persist_write_int(KEY_BACKGROUND, bg_color.argb);
  }

  Tuple *regular_text_t = dict_find(iter, KEY_REGULAR_TEXT);
  if (regular_text_t) {
    ctx->regular_text_color->argb = regular_text_t->value->uint8;
    persist_write_int(KEY_REGULAR_TEXT, ctx->regular_text_color->argb);
  }

  Tuple *bold_text_t = dict_find(iter, KEY_BOLD_TEXT);
  if (bold_text_t) {
    ctx->bold_text_color->argb = bold_text_t->value->uint8;
    persist_write_int(KEY_BOLD_TEXT, ctx->bold_text_color->argb);
  }
#else
  Tuple *color_inverse_t = dict_find(iter, KEY_INVERSE);
  if (color_inverse_t) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Inverse colors is %d", color_inverse_t->value->int8);
    if (color_inverse_t->value->int8 > 0) {
      window_set_background_color(ctx->window, GColorWhite);
      ctx->regular_text_color->argb = GColorBlack.argb;
      ctx->bold_text_color->argb = GColorBlack.argb;
      persist_write_bool(KEY_INVERSE, true);
    } else {
      window_set_background_color(ctx->window, GColorBlack);
      ctx->regular_text_color->argb = GColorWhite.argb;
      ctx->bold_text_color->argb = GColorWhite.argb;
      persist_write_bool(KEY_INVERSE, false);
    }
  }
#endif

  ctx->refresh_time();
}

void config_message_read_persisted_state(const ConfigMessageContext *ctx) {
  if (persist_exists(KEY_LANGUAGE)) {
    ctx->set_language(persist_read_int(KEY_LANGUAGE));
  }
  if (persist_exists(KEY_OFFSET)) {
    ctx->set_offset(persist_read_int(KEY_OFFSET));
  }
  if (persist_exists(KEY_GESTURE)) {
    ctx->set_gesture(persist_read_int(KEY_GESTURE));
  }
  if (persist_exists(KEY_BT_NOTIFICATION)) {
    ctx->set_bt_lost_notification(persist_read_int(KEY_BT_NOTIFICATION));
  }
  if (persist_exists(KEY_STRICT_HOUR_PHRASES)) {
    ctx->set_strict_hour_phrases(persist_read_bool(KEY_STRICT_HOUR_PHRASES));
  } else {
    ctx->set_strict_hour_phrases(true);
  }

  GColor8 background_color;
  background_color.argb = GColorBlack.argb;
  ctx->regular_text_color->argb = GColorWhite.argb;
  ctx->bold_text_color->argb = GColorWhite.argb;

#ifdef PBL_COLOR
  if (persist_exists(KEY_BACKGROUND)) {
    background_color.argb = persist_read_int(KEY_BACKGROUND);
  }
  if (persist_exists(KEY_REGULAR_TEXT)) {
    ctx->regular_text_color->argb = persist_read_int(KEY_REGULAR_TEXT);
  }
  if (persist_exists(KEY_BOLD_TEXT)) {
    ctx->bold_text_color->argb = persist_read_int(KEY_BOLD_TEXT);
  }
#else
  if (persist_exists(KEY_INVERSE) && persist_read_bool(KEY_INVERSE)) {
    background_color.argb = GColorWhite.argb;
    ctx->regular_text_color->argb = GColorBlack.argb;
    ctx->bold_text_color->argb = GColorBlack.argb;
  }
#endif

  window_set_background_color(ctx->window, background_color);
}
