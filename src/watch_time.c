#include "watch_time.h"
#include "fonts.h"

TextLayer* create_time_layer() {
  TextLayer *time_layer = text_layer_create(GRect(0, 20, 144, 60));
  text_layer_set_basic_properties(time_layer, TIME_FONT);
  return time_layer;
}

TextLayer* create_date_layer() {
  TextLayer *layer = text_layer_create(GRect(0, 3, 144, 30));
  text_layer_set_basic_properties(layer, DATUM_FONT);
  return layer;
}

void text_layer_set_basic_properties(TextLayer* layer, GFont font) {
  text_layer_set_background_color(layer, GColorBlack);
  text_layer_set_text_color(layer, GColorClear);
  text_layer_set_text(layer, "");
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
}