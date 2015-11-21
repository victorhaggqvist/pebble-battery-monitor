#include "pebble.h"

#define TEXT_HIDDEN_KEY 1
#define ENABLE_COLOR_KEY 2
//#define PBL_COLOR

Window *window;
TextLayer *battery_percentage;
TextLayer *charge_status;
char percent_show[5];

void battery_state_receiver(BatteryChargeState chargeState){
  uint8_t percent = chargeState.charge_percent;

  snprintf(percent_show, 5, "%d%%", percent);
  text_layer_set_text(battery_percentage, percent_show);

  if (!persist_read_bool(TEXT_HIDDEN_KEY)) {
    if(chargeState.is_plugged && chargeState.is_charging)
      text_layer_set_text(charge_status, "Charging");
    else if(chargeState.is_plugged && !chargeState.is_charging)
      text_layer_set_text(charge_status, "Plugged - Not Charging");
    else if(!chargeState.is_plugged && !chargeState.is_charging)
      text_layer_set_text(charge_status, "Discharging");
  }

#ifdef PBL_COLOR
//  if (persist_read_bool(ENABLE_COLOR_KEY)) {
//    APP_LOG(APP_LOG_LEVEL_DEBUG, "No color setting");
//    return;
//  }



  if (percent > 40) {
    window_set_background_color(window, GColorDarkGreen);
  } else if(percent <=40 && percent > 20) {
    window_set_background_color(window, GColorChromeYellow);
  } else if(percent <= 20) {
    window_set_background_color(window, GColorDarkCandyAppleRed);
  }
#endif
}

void click_handler(ClickRecognizerRef recognizer, void *context) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Single Click");

  if (persist_read_bool(TEXT_HIDDEN_KEY)) {
    persist_write_bool(TEXT_HIDDEN_KEY, false);
    battery_state_receiver(battery_state_service_peek());
  } else {
    persist_write_bool(TEXT_HIDDEN_KEY, true);
    text_layer_set_text(charge_status, "");
  }
}

void long_click_handler(ClickRecognizerRef recognizer, void *context) {

  if (persist_read_bool(ENABLE_COLOR_KEY)) {
    persist_write_bool(ENABLE_COLOR_KEY, false);
    battery_state_receiver(battery_state_service_peek());
  } else {
    persist_write_bool(ENABLE_COLOR_KEY, true);
    battery_state_receiver(battery_state_service_peek());
  }

}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler);
//#ifdef PBL_COLOR
//  window_long_click_subscribe(BUTTON_ID_SELECT, 0, long_click_handler, NULL);
// #endif
}

void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

#ifdef PBL_ROUND
  battery_percentage = text_layer_create((GRect) { .origin = { 0, 55 }, .size = { bounds.size.w, 45 } });
#elif PBL_COLOR
  battery_percentage = text_layer_create((GRect) { .origin = { 0, 45 }, .size = { bounds.size.w, 45 } });
#else
  battery_percentage = text_layer_create((GRect) { .origin = { 0, 32 }, .size = { bounds.size.w, 45 } });
#endif
  text_layer_set_text_alignment(battery_percentage, GTextAlignmentCenter);
  text_layer_set_font(battery_percentage, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_background_color(battery_percentage, GColorClear);

#ifdef PBL_ROUND
  charge_status = text_layer_create((GRect) { .origin = { 0, 102 }, .size = { bounds.size.w, 20 } });
#elif PBL_COLOR
  charge_status = text_layer_create((GRect) { .origin = { 0, 92 }, .size = { bounds.size.w, 20 } });
#else
  charge_status = text_layer_create((GRect) { .origin = { 0, 82 }, .size = { bounds.size.w, 20 } });
#endif
  text_layer_set_text_alignment(charge_status, GTextAlignmentCenter);
  text_layer_set_background_color(charge_status, GColorClear);

  // make a peek to start
  battery_state_receiver(battery_state_service_peek());

  layer_add_child(window_layer, text_layer_get_layer(battery_percentage));
  layer_add_child(window_layer, text_layer_get_layer(charge_status));
}

void window_unload(Window *window) {
  text_layer_destroy(battery_percentage);
  text_layer_destroy(charge_status);
}

void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  battery_state_service_subscribe(battery_state_receiver);

  window_set_click_config_provider(window, click_config_provider);
}

void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
