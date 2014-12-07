#include <pebble.h>
#include <string.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

char *fuzz_past[] = {
  "%s o'clock",
  "five past %s",
  "ten past %s",
  "quarter past %s",
  "twenty past %s",
  "twenty five past %s",
  "half past %s"
};
char *fuzz_to[] = {
  "twenty five to %s",
  "twenty to %s",
  "quarter to %s",
  "ten to %s",
  "five to %s",
  "%s o'clock"
};

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  char* fuzzytext;
  int mins = tick_time->tm_min;
  int hour;
    if (mins > 33)
  {
    hour = tick_time->tm_hour;
    fuzzytext = fuzz_to[(mins - 33)/5];
  }
  else
  {
      hour = tick_time->tm_hour + 1;
    fuzzytext = fuzz_past[(mins - 3)/5];
  }
  
  snprintf(buffer, sizeof(fuzzytext), fuzzytext, hour);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
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
