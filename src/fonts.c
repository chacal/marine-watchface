#include "fonts.h"

GFont UNIT_FONT;
GFont DATUM_FONT;
GFont TIME_FONT;

void load_fonts() {
  UNIT_FONT = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_REGULAR_12));
  DATUM_FONT = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_MEDIUM_20));
  TIME_FONT = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_MEDIUM_56));
}

void unload_fonts() {
  fonts_unload_custom_font(UNIT_FONT);
  fonts_unload_custom_font(DATUM_FONT);
  fonts_unload_custom_font(TIME_FONT);
}