#include <pebble.h>
#include <math.h>
#define MAX_TEXT_LENGTH sizeof("twenty\nfive\npast\ntwelve")
#define UPDATE_TIME_NOW() time_t _t = time(NULL);\
    update_time(localtime(&_t))
typedef struct tm time_s;

    
static Window *s_main_window;
static TextLayer *s_time_layer;
static bool shaken = false;


// Create a long-lived buffer
static char buffer[MAX_TEXT_LENGTH];

static const char units[][MAX_TEXT_LENGTH] = {
    "o'clock",
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
    "twelve",
    "thirteen",
    "fourteen",
    "fifteen",
    "sixteen",
    "seventeen",
    "eighteen",
    "ninteen"       
};

static const char tens[][MAX_TEXT_LENGTH] = {
    "",
    "",
    "twenty",
    "thirty",
    "forty",
    "fifty",
};


static char* time_to_exact_string(const time_s* time)
{
    int mins = time->tm_min;
    int mins_units = mins % 10;
    int mins_tens = mins / 10;
    
    int idx = 0;
    
    const char* time_parts[3] = {"","",""};
    
    if (time->tm_min < 20)
    {
        time_parts[idx++] = units[time->tm_min];
    }
    else if (mins_units == 0)
    {
        time_parts[idx++] = tens[mins_tens];
    }
    else
    {
        time_parts[idx++] = units[mins_units];
        time_parts[idx++] = tens[mins_tens];
    }
    
    int hour = time->tm_hour % 12;
    hour = hour == 0 ? 12 : hour;
    
    time_parts[idx] = units[hour];
    
    snprintf(buffer, MAX_TEXT_LENGTH, "\n%s\n%s\n%s", time_parts[2], time_parts[1],time_parts[0]);
    
    return buffer;
}

static const char fuzzy_mins[][MAX_TEXT_LENGTH] = {
    "\n\n%s\no'clock",
    "\nfive\npast\n%s",
    "\nten\npast\n%s",
    "\nquarter\npast\n%s",
    "\ntwenty\npast\n%s",
    "twenty\nfive\npast\n%s",
    "\nhalf\npast\n%s",
    "twenty\nfive\nto\n%s",
    "\ntwenty\nto\n%s",
    "\nquarter\nto\n%s",
    "\nten\nto\n%s",
    "\nfive\nto\n%s",
    "\n\n%s\no'clock"
};

static char* time_to_fuzzy_string(const time_s* time)
{
    int mins = time->tm_min;
    int hour = time->tm_hour;
    
    // convert minutes to nearest five
    int idx = (mins + 2) / 5; 
    
    // correct "mins to" times;
    if (idx > 6)
    {
        hour += 1;
    }
    
    // AM/PM is irrelevant, also adjust for above taking out of range)
    hour %= 12;
    hour = hour == 0 ? 12 : hour;
    
    // combine minute and hour
    snprintf(buffer, MAX_TEXT_LENGTH, fuzzy_mins[idx], units[hour]);
  
    return buffer;
}


static void update_time(const time_s *tick_time) 
{
    
    if (!shaken)
    {
        text_layer_set_text(s_time_layer, time_to_fuzzy_string(tick_time));
    }
    else
    {
        text_layer_set_text(s_time_layer, time_to_exact_string(tick_time));
    }
}

static void main_window_load(Window *window) 
{
    // Create time TextLayer
    s_time_layer = text_layer_create(GRect(0, -2, 144, 170));
    text_layer_set_background_color(s_time_layer, GColorBlack);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");

    // Format text layer
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
    text_layer_set_overflow_mode(s_time_layer, GTextOverflowModeWordWrap);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
    // Make sure the time is displayed from the start
    UPDATE_TIME_NOW();
}

static void main_window_unload(Window *window)
{
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
}

static void tick_handler(time_s *tick_time, TimeUnits units_changed)
{
    // if not showing exact time, reduce battery usage by only updating screen when required
    if (shaken || tick_time->tm_min % 5 == 3) 
        update_time(tick_time);
}

static void unshake(void * data)
{
    shaken = false;
    UPDATE_TIME_NOW();
}

static void tap_handler(AccelAxisType axis, int32_t direction)
{
    shaken = true;
    UPDATE_TIME_NOW();
    app_timer_register(2000, (AppTimerCallback)unshake, NULL);	
}


static void init() 
{
    // Create main Window element and assign to pointer
    s_main_window = window_create();
    window_set_fullscreen(s_main_window, true);
    
    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    
    accel_tap_service_subscribe((AccelTapHandler)tap_handler);

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);
  
    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() 
{
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) 
{
    init();
    app_event_loop();
    deinit();
}