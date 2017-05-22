#include "watch_time.h"
#include "fonts.h"
#include "utils.h"

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
