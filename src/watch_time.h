#pragma once
#include <pebble.h>

TextLayer* create_time_layer();
TextLayer* create_date_layer();

void text_layer_set_basic_properties(TextLayer* layer, GFont font);