#include <pebble.h>
#include "watch_time.h"
#include "fonts.h"
#include "forecast.h"

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_wind_speed_layer;
static TextLayer *s_wind_speed_unit_layer;
static TextLayer *s_wind_dir_layer;
static TextLayer *s_wind_dir_unit_layer;
static TextLayer *s_temperature_layer;
static TextLayer *s_temperature_unit_layer;
static TextLayer *s_observation_station_layer;
static TextLayer *s_last_update_layer;
static int s_battery_level;
static bool s_bt_connected;
static bool s_phone_ready;

static char s_wind_speed_buf[16] = "-";
static char s_wind_dir_buf[16] = "-";
static char s_temperature_buf[16] = "-";
static char s_observation_station_buf[128] = "-";
static char s_last_update_buf[16] = "-";

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

static void update_observation(const uint32_t key) {
  if(key == MESSAGE_KEY_WindSpeed) {
    persist_read_string(MESSAGE_KEY_WindSpeed, s_wind_speed_buf, 16);
    text_layer_set_text(s_wind_speed_layer, s_wind_speed_buf);
  } else if(key == MESSAGE_KEY_WindDir) {
    persist_read_string(MESSAGE_KEY_WindDir, s_wind_dir_buf, 16);
    text_layer_set_text(s_wind_dir_layer, s_wind_dir_buf);
  } else if(key == MESSAGE_KEY_Temperature) {
    persist_read_string(MESSAGE_KEY_Temperature, s_temperature_buf, 16);
    text_layer_set_text(s_temperature_layer, s_temperature_buf);
  } else if(key == MESSAGE_KEY_ObservationStation) {
    persist_read_string(MESSAGE_KEY_ObservationStation, s_observation_station_buf, 128);
    text_layer_set_text(s_observation_station_layer, s_observation_station_buf);
  } else if(key == MESSAGE_KEY_LastObservationUpdate) {
    int32_t last_update = persist_read_int(MESSAGE_KEY_LastObservationUpdate);
    struct tm *time = localtime(&last_update);
    strftime(s_last_update_buf, 16, "%H:%M", time);
    text_layer_set_text(s_last_update_layer, s_last_update_buf);
  }
}

static void request_observations() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting new observations..");

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
  if(key == MESSAGE_KEY_Ready && strcmp(new_tuple->value->cstring, "ready") == 0 && !s_phone_ready) {
    s_phone_ready = true;
    if(time(NULL) - persist_read_int(MESSAGE_KEY_LastObservationUpdate) > 5 * 60) {
      request_observations();
    }
  }

  if(key == MESSAGE_KEY_LastObservationUpdate && new_tuple->value->uint32 != 0) {
    persist_write_int(MESSAGE_KEY_LastObservationUpdate, new_tuple->value->uint32);
  } else if(strcmp(new_tuple->value->cstring, "") != 0) {
    persist_write_string(key, new_tuple->value->cstring);
  }
  update_observation(key);
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

  if(s_bt_connected) {
    GPoint bt_origin = GPoint(135, 168-3);
    graphics_draw_line(ctx, GPoint(bt_origin.x + 3, bt_origin.y - 12), GPoint(bt_origin.x + 3, bt_origin.y));
    graphics_draw_line(ctx, GPoint(bt_origin.x + 3, bt_origin.y - 12), GPoint(bt_origin.x + 6, bt_origin.y - 9));
    graphics_draw_line(ctx, GPoint(bt_origin.x, bt_origin.y - 3), GPoint(bt_origin.x + 6, bt_origin.y - 9));

    graphics_draw_line(ctx, GPoint(bt_origin.x, bt_origin.y - 9), GPoint(bt_origin.x + 6, bt_origin.y - 3));
    graphics_draw_line(ctx, GPoint(bt_origin.x + 3, bt_origin.y), GPoint(bt_origin.x + 6, bt_origin.y - 3));
  }
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

  s_wind_speed_layer = create_forecast_data_layer(GRect(0, 99, 48, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_wind_speed_layer));

  s_wind_speed_unit_layer = create_forecast_unit_layer(GRect(0, 99+22, 48, 20));
  text_layer_set_text(s_wind_speed_unit_layer, "m/s");
  layer_add_child(window_layer, text_layer_get_layer(s_wind_speed_unit_layer));

  s_wind_dir_layer = create_forecast_data_layer(GRect(48, 99, 48, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_wind_dir_layer));

  s_wind_dir_unit_layer = create_forecast_unit_layer(GRect(48, 99+22, 48, 20));
  text_layer_set_text(s_wind_dir_unit_layer, "°T");
  layer_add_child(window_layer, text_layer_get_layer(s_wind_dir_unit_layer));

  s_temperature_layer = create_forecast_data_layer(GRect(96, 99, 48, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));

  s_temperature_unit_layer = create_forecast_unit_layer(GRect(96, 99+22, 48, 20));
  text_layer_set_text(s_temperature_unit_layer, "°C");
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_unit_layer));

  s_observation_station_layer = create_forecast_unit_layer(GRect(0, 137, 144, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_observation_station_layer));

  s_last_update_layer = create_forecast_unit_layer(GRect(0, 152, 144, 20));
  layer_add_child(window_layer, text_layer_get_layer(s_last_update_layer));

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  Tuplet initial_values[] = {
      TupletCString(MESSAGE_KEY_Ready, ""),
      TupletInteger(MESSAGE_KEY_LastObservationUpdate, 0),
      TupletCString(MESSAGE_KEY_WindSpeed, ""),
      TupletCString(MESSAGE_KEY_WindDir, ""),
      TupletCString(MESSAGE_KEY_Temperature, ""),
      TupletCString(MESSAGE_KEY_ObservationStation, "")
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
                initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL
  );
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_wind_speed_layer);
  text_layer_destroy(s_wind_speed_unit_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_temperature_unit_layer);
  text_layer_destroy(s_wind_dir_layer);
  text_layer_destroy(s_wind_dir_unit_layer);
  text_layer_destroy(s_observation_station_layer);
  text_layer_destroy(s_last_update_layer);
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

static void bluetooth_callback(bool connected) {
  s_bt_connected = connected;
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
  battery_callback(battery_state_service_peek());
  connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bluetooth_callback
  });
  bluetooth_callback(connection_service_peek_pebble_app_connection());
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