#include <pebble.h>

#define TEXT_HIDDEN_KEY 1

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

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler);
}

void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  battery_percentage = text_layer_create((GRect) { .origin = { 0, 32 }, .size = { bounds.size.w, 45 } });
  text_layer_set_text_alignment(battery_percentage, GTextAlignmentCenter);
  text_layer_set_font(battery_percentage, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  charge_status = text_layer_create((GRect) { .origin = { 0, 82 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_alignment(charge_status, GTextAlignmentCenter);

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
