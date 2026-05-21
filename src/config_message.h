#pragma once

#include <pebble.h>

typedef struct {
  Window *window;
  GColor8 *regular_text_color;
  GColor8 *bold_text_color;
  void (*set_language)(uint8_t language);
  void (*set_offset)(int offset);
  void (*set_gesture)(int gesture);
  void (*set_bt_lost_notification)(int bt_notification);
  void (*set_strict_hour_phrases)(bool enabled);
  void (*refresh_time)(void);
} ConfigMessageContext;

void config_message_handle_inbox(DictionaryIterator *iter, const ConfigMessageContext *ctx);
void config_message_read_persisted_state(const ConfigMessageContext *ctx);
