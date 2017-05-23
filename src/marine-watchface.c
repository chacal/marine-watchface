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
static int s_battery_level;

#define APP_SYNC_BUFFER_SIZE 128
static AppSync s_sync;
static uint8_t s_sync_buffer[APP_SYNC_BUFFER_SIZE];

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

static void request_observations() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to sen App Message to phone!");
    return;
  }

  int value = 1;
  dict_write_int(iter, 0, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  if(key == MESSAGE_KEY_Wind) {
    text_layer_set_text(s_wind_layer, new_tuple->value->cstring);
  } else if(key == MESSAGE_KEY_Temperature) {
    text_layer_set_text(s_temperature_layer, new_tuple->value->cstring);
  } else if(key == MESSAGE_KEY_Pressure) {
    text_layer_set_text(s_pressure_layer, new_tuple->value->cstring);
  } else if(key == MESSAGE_KEY_ObservationStation) {
    text_layer_set_text(s_observation_station_layer, new_tuple->value->cstring);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorClear);
  graphics_context_set_fill_color(ctx, GColorClear);

  graphics_draw_line(ctx, GPoint(0, 88), GPoint(144, 88));

  int battery_height = s_battery_level / 10;
  GPoint battery_origin = GPoint(2, 168-2);
  graphics_draw_rect(ctx, GRect(battery_origin.x, battery_origin.y - 12, 4, 12));
  graphics_draw_rect(ctx, GRect(battery_origin.x + 1, battery_origin.y - 13, 2, 1));
  graphics_fill_rect(ctx, GRect(battery_origin.x + 1, battery_origin.y - 1 - battery_height, 2, battery_height), 0, GCornerNone);
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

  s_temperature_layer = create_forecast_data_layer(GRect(57, 99, 30, 20));
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

  Tuplet initial_values[] = {
      TupletInteger(MESSAGE_KEY_Ready, (uint8_t)0),
      TupletCString(MESSAGE_KEY_Wind, "-"),
      TupletCString(MESSAGE_KEY_Temperature, "-"),
      TupletCString(MESSAGE_KEY_Pressure, "-"),
      TupletCString(MESSAGE_KEY_ObservationStation, "-")
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
                initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL
  );
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
  if(tick_time->tm_min % 15 == 0) {
    request_observations();
  }
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_canvas_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  update_time();

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
  app_message_open(APP_SYNC_BUFFER_SIZE, APP_SYNC_BUFFER_SIZE);
}

static void deinit() {
  window_destroy(s_main_window);
  app_sync_deinit(&s_sync);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}