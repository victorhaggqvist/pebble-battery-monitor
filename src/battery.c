#include <pebble.h>
#include <inttypes.h>

static Window *window;
static TextLayer *battery_percentage;
static TextLayer *charge_status;
static BatteryChargeState chargeState;
static char percent_show[5];

static void battery_state_receiver(BatteryChargeState chargeState){
  
  uint8_t percent = chargeState.charge_percent;
  
  snprintf(percent_show, 5, "%i%%", percent);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "percent: %s", percent_show);
  text_layer_set_text(battery_percentage, percent_show);
  
  if(chargeState.is_charging){
    text_layer_set_text(charge_status, "Charging");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "charge state: charging");
  }
  else{
    text_layer_set_text(charge_status, "Discharging");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "charge state: discharging");
  }
  
  if(chargeState.is_plugged)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "charge state: plugged");
  else
    APP_LOG(APP_LOG_LEVEL_DEBUG, "charge state: unplugged");
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  battery_percentage = text_layer_create((GRect) { .origin = { 0, 52 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_alignment(battery_percentage, GTextAlignmentCenter);
  
  charge_status = text_layer_create((GRect) { .origin = { 0, 82 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_alignment(charge_status, GTextAlignmentCenter);
  
  chargeState = battery_state_service_peek();
  battery_state_receiver(chargeState);
  
  layer_add_child(window_layer, text_layer_get_layer(battery_percentage));
  layer_add_child(window_layer, text_layer_get_layer(charge_status));
}

static void window_unload(Window *window) {
  text_layer_destroy(battery_percentage);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  battery_state_service_subscribe(battery_state_receiver);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
