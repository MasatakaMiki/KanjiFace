#include <pebble.h>
#include "main.h"

#include <stdlib.h>
#include <stdio.h>

static Window *s_main_window;
static BitmapLayer *s_background_layer, *s_bt_icon_layer, *s_date_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap, *s_date_icon_bitmap;
static TextLayer *s_battery_layer;
static TextLayer *s_hour_layer, *s_min_layer;
static TextLayer *s_month_layer, *s_day_layer;
static GFont s_battery_font;
static GFont s_time_font;
static GFont s_date_font;
static char s_battery_buffer[16];

// A struct for our specific settings (see main.h)
ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {
  snprintf(settings.Like_kanji, sizeof(settings.Like_kanji), "%s", "YID");
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void battery_handler(BatteryChargeState charge_state) {
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
  if(charge_state.charge_percent <= 20) {
    text_layer_set_text_color(s_battery_layer, GColorRed);
  } else if(charge_state.charge_percent <= 50) {
    text_layer_set_text_color(s_battery_layer, GColorChromeYellow);
  } else {
    text_layer_set_text_color(s_battery_layer, GColorBlack);    
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_hour_buffer[4];
  static char s_min_buffer[4];
  strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_min_buffer, sizeof(s_min_buffer), "%M", tick_time);
  static char s_month_buffer[4];
  static char s_day_buffer[4];
  strftime(s_month_buffer, sizeof(s_month_buffer), "%m", tick_time);
  strftime(s_day_buffer, sizeof(s_day_buffer), "%d", tick_time);
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%A", tick_time);

  // cut off 0
  snprintf(s_hour_buffer, 4, "%d", atoi(s_hour_buffer));
  snprintf(s_min_buffer, 4, "%d", atoi(s_min_buffer));
  snprintf(s_month_buffer, 4, "%d", atoi(s_month_buffer));
  snprintf(s_day_buffer, 4, "%d", atoi(s_day_buffer));

  // Display this time on the TextLayer
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_min_layer, s_min_buffer);
  text_layer_set_text(s_month_layer, s_month_buffer);
  text_layer_set_text(s_day_layer, s_day_buffer);

  // for check of character positions
  /*
  text_layer_set_text(s_hour_layer, "99");
  text_layer_set_text(s_min_layer, "99");
  text_layer_set_text(s_month_layer, "99");
  text_layer_set_text(s_day_layer, "99");
  */

  // Date image
  if (strcmp(s_date_buffer, "Sunday") == 0) {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_SUN);
  } else if (strcmp(s_date_buffer, "Monday") == 0) {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_MON);
  } else if (strcmp(s_date_buffer, "Tuesday") == 0) {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_TUE);
  } else if (strcmp(s_date_buffer, "Wednesday") == 0) {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_WED);
  } else if (strcmp(s_date_buffer, "Thursday") == 0) {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_THU);
  } else if (strcmp(s_date_buffer, "Friday") == 0) {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_FRI);
  } else if (strcmp(s_date_buffer, "Saturday") == 0) {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_SAT);
  } else {
    s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_DEF);
  }  
  bitmap_layer_set_bitmap(s_date_icon_layer, s_date_icon_bitmap);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_DISCONN);
  s_date_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DATE_DEF);
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  // date
  s_date_icon_layer = bitmap_layer_create(GRect(100, 139, 34, 24));
  bitmap_layer_set_bitmap(s_date_icon_layer, s_date_icon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_date_icon_layer));
  // bluetooth
  s_bt_icon_layer = bitmap_layer_create(GRect(10, 12, 18, 18));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_icon_layer));

  // Create the TextLayer with specific bounds
  s_battery_layer = text_layer_create(GRect(6, 5, bounds.size.w - 20, 18));
  s_hour_layer = text_layer_create(GRect(72, 13, 54, 54));
  s_min_layer = text_layer_create(GRect(12, 30, 54, 54));
  s_month_layer = text_layer_create(GRect(14, 138, 24, 24));
  s_day_layer = text_layer_create(GRect(56, 138, 24, 24));
  // background color
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_background_color(s_min_layer, GColorClear);
  text_layer_set_background_color(s_month_layer, GColorClear);
  text_layer_set_background_color(s_day_layer, GColorClear);
  // forecolor
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text_color(s_hour_layer, GColorBlack);
  text_layer_set_text_color(s_min_layer, GColorBlack);
  text_layer_set_text_color(s_month_layer, GColorBlack);
  text_layer_set_text_color(s_day_layer, GColorBlack);
  // font
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_14));
  text_layer_set_font(s_battery_layer, s_battery_font);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_G_ROUND_BOLD_KANA_48));
  text_layer_set_font(s_hour_layer, s_time_font);
  text_layer_set_font(s_min_layer, s_time_font);
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_G_ROUND_BOLD_KANA_20));
  text_layer_set_font(s_month_layer, s_date_font);
  text_layer_set_font(s_day_layer, s_date_font);
  // alignment
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_min_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);

  // Battery
  BatteryChargeState charge_state = battery_state_service_peek();
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
  if (charge_state.charge_percent <= 20) {
    text_layer_set_text_color(s_battery_layer, GColorRed);
  } else if (charge_state.charge_percent <= 50) {
    text_layer_set_text_color(s_battery_layer, GColorChromeYellow);
  } else {
    text_layer_set_text_color(s_battery_layer, GColorBlack);    
  }
  // Time
  text_layer_set_text(s_hour_layer, "00");
  text_layer_set_text(s_min_layer, "00");
  // Date
  text_layer_set_text(s_month_layer, "10");
  text_layer_set_text(s_day_layer, "26");

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_min_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_month_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));

  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());

  battery_state_service_subscribe(battery_handler);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_min_layer);
  text_layer_destroy(s_month_layer);
  text_layer_destroy(s_day_layer);
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_bt_icon_bitmap);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_date_icon_layer);
  bitmap_layer_destroy(s_bt_icon_layer);
  // Unload GFont
  fonts_unload_custom_font(s_battery_font);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  // Battery State Service
  battery_state_service_unsubscribe();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read config
  Tuple *like_kanji_tuple = dict_find(iterator, MESSAGE_KEY_LIKE_KANJI);
  if (like_kanji_tuple) {
    snprintf(settings.Like_kanji, sizeof(settings.Like_kanji), "%s", like_kanji_tuple->value->cstring);
    prv_save_settings();
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  prv_load_settings();

  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
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