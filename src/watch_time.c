#include "watch_time.h"

TextLayer* create_time_layer(GFont s_time_font) {
  // Create the TextLayer with specific bounds
  TextLayer *s_time_layer = text_layer_create(GRect(0, 52, 144, 60));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  return s_time_layer;
}
