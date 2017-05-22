#include "fonts.h"

GFont TIME_FONT;

void load_fonts() {
  TIME_FONT = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_MEDIUM_56));
}

void unload_fonts() {
  fonts_unload_custom_font(TIME_FONT);
}