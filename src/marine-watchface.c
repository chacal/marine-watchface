#include <pebble.h>
#include "watch_time.h"
#include "fonts.h"
#include "forecast.h"

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_wind_layer;
static TextLayer *s_wind_unit_layer;
static TextLayer *s_temperature_layer;
static TextLayer *s_temperature_unit_layer;
static TextLayer *s_pressure_layer;
static TextLayer *s_pressure_unit_layer;
static TextLayer *s_observation_station_layer;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);

  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), tick_time->tm_mday < 10 ? "%a,%e %b" : "%a, %d %b", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorClear);
  graphics_draw_line(ctx, GPoint(0, 88), GPoint(144, 88));
}

static void main_window_load(Window *window) {
  load_fonts();
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_layer = create_time_layer();
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_layer = create_date_layer();
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_wind_layer = create_forecast_data_layer(GRect(0, 99, 58, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_wind_layer));

  s_wind_unit_layer = create_forecast_unit_layer(GRect(7, 99+22, 48, 20));
  text_layer_set_text(s_wind_unit_layer, "m/s");
  layer_add_child(window_layer, text_layer_get_layer(s_wind_unit_layer));

  s_temperature_layer = create_forecast_data_layer(GRect(53, 99, 38, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));

  s_temperature_unit_layer = create_forecast_unit_layer(GRect(48, 99+22, 48, 20));
  text_layer_set_text(s_temperature_unit_layer, "°C");
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_unit_layer));

  s_pressure_layer = create_forecast_data_layer(GRect(93, 99, 48, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_pressure_layer));

  s_pressure_unit_layer = create_forecast_unit_layer(GRect(94, 99+22, 48, 20));
  text_layer_set_text(s_pressure_unit_layer, "mbar");
  layer_add_child(window_layer, text_layer_get_layer(s_pressure_unit_layer));

  s_observation_station_layer = create_forecast_unit_layer(GRect(0, 142, 144, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_observation_station_layer));

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_wind_layer);
  text_layer_destroy(s_wind_unit_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_temperature_unit_layer);
  text_layer_destroy(s_pressure_layer);
  text_layer_destroy(s_pressure_unit_layer);
  text_layer_destroy(s_observation_station_layer);
  layer_destroy(s_canvas_layer);
  unload_fonts();
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}