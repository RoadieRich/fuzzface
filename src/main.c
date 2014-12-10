#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

// Create a long-lived buffer
static char buffer[] = "twenty five past twelve";
static char hourStr[][7] = {
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven",
    "twelve"
};

static void update_time(struct tm *tick_time) {

    
    char fuzzytext[20];
    int mins = tick_time->tm_min;
    int hour = tick_time->tm_hour;
    
    switch(mins){
                          case  0: case  1: case  2: 
            strcpy(fuzzytext, "\n\n%s\no'clock");
            break;
        case  3: case  4: case  5: case  6: case  7: 
            strcpy(fuzzytext, "\nfive\npast\n%s");
            break;
        case  8: case  9: case 10: case 11: case 12: 
            strcpy(fuzzytext, "\nten\npast\n%s");
            break;
        case 13: case 14: case 15: case 16: case 17: 
            strcpy(fuzzytext, "\nquarter\npast\n%s");
            break;
        case 18: case 19: case 20: case 21: case 22: 
            strcpy(fuzzytext, "\ntwenty\npast\n%s");
        case 23: case 24: case 25: case 26: case 27: 
            strcpy(fuzzytext, "twenty\nfive\npast\n%s");
            break;
        case 28: case 29: case 30: case 31: case 32: 
            strcpy(fuzzytext, "\nhalf\npast\n%s");
            break;
        case 33: case 34: case 35: case 36: case 37: 
            strcpy(fuzzytext, "twenty\nfive\nto\n%s");
            hour += 1;
            break;
        case 38: case 39: case 40: case 41: case 42: 
            strcpy(fuzzytext, "\ntwenty\nto\n%s");
            hour += 1;
            break;
        case 43: case 44: case 45: case 46: case 47: 
            strcpy(fuzzytext, "\nquarter\nto\n%s");
            hour += 1;
            break;
        case 48: case 49: case 50: case 51: case 52: 
            strcpy(fuzzytext, "\nten\nto\n%s");
            hour += 1;
            break;
        case 53: case 54: case 55: case 56: case 57: 
            strcpy(fuzzytext, "five\nto\n%s");
            hour += 1;
            break;
        case 58: case 59:
            strcpy(fuzzytext, "\n\n%s\no'clock");
            hour += 1;
            break;
        default:
            strcpy(fuzzytext, "invalid time");
            break;
    }
    
    if(hour > 12)
    {
        hour -= 12;
    }
    
    char * hourText = hourStr[hour-1];
    
    snprintf(buffer, strlen(fuzzytext)+strlen(hourText), fuzzytext, hourText);
  
    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
    // Create time TextLayer
    s_time_layer = text_layer_create(GRect(0, -2, 144, 170));
    text_layer_set_background_color(s_time_layer, GColorBlack);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");

    // Improve the layout to be more like a watchface
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
    text_layer_set_overflow_mode(s_time_layer, GTextOverflowModeWordWrap);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
    // Make sure the time is displayed from the start
    time_t t = time(NULL);
    update_time(localtime(&t));
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
}

static void init() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();
    window_set_fullscreen(s_main_window, true);
    
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
