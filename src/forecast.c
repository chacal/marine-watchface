#include "forecast.h"
#include "utils.h"
#include "fonts.h"

TextLayer* create_forecast_data_layer(GRect bounds) {
  TextLayer *layer = text_layer_create(bounds);
  text_layer_set_basic_properties(layer, DATUM_FONT);
  return layer;
}

TextLayer* create_forecast_unit_layer(GRect bounds) {
  TextLayer *layer = text_layer_create(bounds);
  text_layer_set_basic_properties(layer, UNIT_FONT);
  return layer;
}