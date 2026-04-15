#include <pebble.h>
#include "WaterScreen.h"
#include "AppState.h"
#include "TimerScreen.h"
#include "UIContraints.h"

static Window *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_value_layer;
static TextLayer *s_instruction_layer;
static TextLayer *s_header_layer;
static TextLayer *s_header_container_layer;
static TextLayer *s_bottom_header_layer;
static TextLayer *s_water_header_layer;
static TextLayer *s_water_value_layer;
static TextLayer *s_coffee_header_layer;
static TextLayer *s_coffee_value_layer;

#define waterOzingrams 28.3495
#define HEADER_HEIGHT PBL_IF_ROUND_ELSE(40, HEADER_HEIGHT_RECT)
#define HEADER_PADDING PBL_IF_ROUND_ELSE(0, HEADER_HEIGHT_RECT)
#define CENTER_V_OFFSET 30

// Forward declaration
static void update_display(void);

// logic for determining coffee grams based on water oz and roast level
static float caulculate_coffee_grams(AppState *state, int water_oz)
{
    RoastLevel roast = state->roast_level;
    switch (roast)
    {
    case ROAST_LIGHT:
        return (water_oz * waterOzingrams) / 12;
    case ROAST_MEDIUM_LIGHT:
        return (water_oz * waterOzingrams) / 13.5;
    case ROAST_MEDIUM:
        return (water_oz * waterOzingrams) / 15;
    case ROAST_MEDIUM_DARK:
        return (water_oz * waterOzingrams) / 16.5;
    case ROAST_DARK:
        return (water_oz * waterOzingrams) / 18;
    default:
        return (water_oz * waterOzingrams) / 15;
    }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
    app_state_set_screen(APP_SCREEN_TIMER);
    timer_screen_push();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    AppState *state = app_state_get();
    state->water_oz += 1;
    // Recalculate coffee: approximately 1.125g per oz (18g for 16oz)
    state->coffee_grams = (caulculate_coffee_grams(state, state->water_oz));
    update_display();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    AppState *state = app_state_get();
    if (state->water_oz > 0)
    {
        state->water_oz -= 1;
        // Recalculate coffee: approximately 1.125g per oz (18g for 16oz)
        state->coffee_grams = (caulculate_coffee_grams(state, state->water_oz));
        update_display();
    }
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void update_display(void)
{
    AppState *state = app_state_get();
    static char water_text[64];
    static char coffee_text[64];

    snprintf(water_text, sizeof(water_text), "%doz", state->water_oz);
    snprintf(coffee_text, sizeof(coffee_text), "%dg", state->coffee_grams);
 
    text_layer_set_text(s_water_value_layer, water_text);
    text_layer_set_text(s_coffee_value_layer, coffee_text);

    
}

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    window_set_background_color(window, GColorWindsorTan);

    s_header_container_layer = text_layer_create(GRect(0, 0, bounds.size.w, HEADER_HEIGHT));
    text_layer_set_background_color(s_header_container_layer, GColorBlack);
    int font_height = 30;

    // fixed header layer
    s_header_layer = text_layer_create(GRect(0, HEADER_HEIGHT - font_height, bounds.size.w, font_height));
    text_layer_set_text(s_header_layer, "Brew Amounts");
    text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_background_color(s_header_layer, GColorBlack);
    text_layer_set_text_color(s_header_layer, GColorWhite);
    text_layer_set_overflow_mode(s_header_layer, GTextOverflowModeTrailingEllipsis);

    //water header layer 
    s_water_header_layer = text_layer_create(GRect(0, bounds.size.h/2 - font_height/2 - CENTER_V_OFFSET, bounds.size.w/2, font_height));
    text_layer_set_text(s_water_header_layer, "Water");
    text_layer_set_text_alignment(s_water_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_water_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_water_header_layer, GColorWhite);
    text_layer_set_background_color(s_water_header_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_water_header_layer));

    //coffee header layer 
    s_coffee_header_layer = text_layer_create(GRect(bounds.size.w/2, bounds.size.h/2 - font_height/2 - CENTER_V_OFFSET, bounds.size.w/2, font_height));
    text_layer_set_text(s_coffee_header_layer, "Coffee");
    text_layer_set_text_alignment(s_coffee_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_coffee_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_coffee_header_layer, GColorWhite);
    text_layer_set_background_color(s_coffee_header_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_coffee_header_layer));


    //water value layer 
    s_water_value_layer = text_layer_create(GRect(0, bounds.size.h/2 - font_height/2 + CENTER_V_OFFSET, bounds.size.w/2, font_height));
    text_layer_set_text_alignment(s_water_value_layer, GTextAlignmentCenter);
    text_layer_set_font(s_water_value_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_water_value_layer, GColorWhite);
    text_layer_set_background_color(s_water_value_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_water_value_layer));

    //coffee value layer 
    s_coffee_value_layer = text_layer_create(GRect(bounds.size.w/2, bounds.size.h/2 - font_height/2 + CENTER_V_OFFSET, bounds.size.w/2, font_height));
    text_layer_set_text_alignment(s_coffee_value_layer, GTextAlignmentCenter);
    text_layer_set_font(s_coffee_value_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_coffee_value_layer, GColorWhite);
    text_layer_set_background_color(s_coffee_value_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_coffee_value_layer));

    s_bottom_header_layer = text_layer_create(GRect(0, bounds.size.h - HEADER_HEIGHT, bounds.size.w, HEADER_HEIGHT));
    text_layer_set_text(s_bottom_header_layer, "Based on Roast Level");
    text_layer_set_text_alignment(s_bottom_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_bottom_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_background_color(s_bottom_header_layer, GColorBlack);
    text_layer_set_text_color(s_bottom_header_layer, GColorWhite);
    text_layer_set_overflow_mode(s_bottom_header_layer, GTextOverflowModeTrailingEllipsis);
    layer_add_child(window_layer, text_layer_get_layer(s_bottom_header_layer));

    layer_add_child(window_layer, text_layer_get_layer(s_header_container_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

    update_display();
}

static void window_unload(Window *window)
{
    text_layer_destroy(s_title_layer);
    text_layer_destroy(s_value_layer);
    text_layer_destroy(s_instruction_layer);
    text_layer_destroy(s_header_layer);
    text_layer_destroy(s_header_container_layer);
}

void water_screen_push(void)
{
    AppState *state = app_state_get();
    state->coffee_grams = (caulculate_coffee_grams(state, state->water_oz));
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers){
                                             .load = window_load,
                                             .unload = window_unload,
                                         });
    window_stack_push(s_window, true);
}
