#include "watch_time.h"
#include "fonts.h"

TextLayer* create_time_layer() {
  // Create the TextLayer with specific bounds
  TextLayer *time_layer = text_layer_create(GRect(0, 52, 144, 60));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_font(time_layer, TIME_FONT);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

  return time_layer;
}
