#include "utils.h"

void text_layer_set_basic_properties(TextLayer* layer, GFont font) {
  text_layer_set_background_color(layer, GColorBlack);
  text_layer_set_text_color(layer, GColorClear);
  text_layer_set_text(layer, "");
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
}