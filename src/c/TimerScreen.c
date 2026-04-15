#include <pebble.h>
#include "TimerScreen.h"
#include "AppState.h"
#include "RoastScreen.h"
#include "UIContraints.h"

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer *s_title_layer;
static TextLayer *s_timer_layer;
static TextLayer *s_instruction_layer;
static TextLayer *s_header_layer;
static TextLayer *s_header_container_layer;
static AppTimer *s_timer;
static bool s_timer_running = false;
static int s_remaining_seconds = 0;
static int s_display_min = 0;
static int s_display_sec = 0;
static char s_timer_buffer[12];

#define HEADER_HEIGHT PBL_IF_ROUND_ELSE(40, HEADER_HEIGHT_RECT)
#define HEADER_PADDING PBL_IF_ROUND_ELSE(0, HEADER_HEIGHT_RECT)

// Forward declaration
static void update_display(void *context);

// static void select_click_handler(ClickRecognizerRef recognizer, void *context)
// {
//     if (!s_timer_running)
//     {
//         // Start the timer
//         AppState *state = app_state_get();
//         s_remaining_seconds = state->brew_time_seconds;
//         s_timer = app_timer_register(1000, update_display, NULL); // Fire every 1 second
//         s_timer_running = true;
//         update_display(NULL);  // Update display immediately
//     }
//     else
//     {
//         // Stop the timer
//         app_timer_cancel(s_timer);
//         s_timer_running = false;
//         AppState *state = app_state_get();
//         s_remaining_seconds = state->brew_time_seconds;
//         update_display(NULL);
//     }
// }

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
    // app_state_set_screen(APP_SCREEN_ROAST);
    // roast_screen_push();
    if (!s_timer_running)
    {
        if (s_remaining_seconds == 0)
        {
            AppState *state = app_state_get();
            s_remaining_seconds = state->brew_time_seconds;
            update_display(NULL);
            // window_stack_pop_all(true);
        }
        else
        {
            s_remaining_seconds = app_state_get()->brew_time_seconds;
            s_timer = app_timer_register(1000, update_display, NULL); // Fire every 1 second
            s_timer_running = true;
        }
    }
    else
    {
        app_timer_cancel(s_timer);
        s_timer_running = false;
        s_remaining_seconds = app_state_get()->brew_time_seconds;
        update_display(NULL);
    }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    AppState *state = app_state_get();
    state->brew_time_seconds += 15; // Increase by 15 seconds
    update_display(NULL);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    AppState *state = app_state_get();
    if (state->brew_time_seconds > 15)
    {
        state->brew_time_seconds -= 15; // Decrease by 15 seconds
        update_display(NULL);
    }
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/* OLD
static void update_canvas_layer(Layer *layer, GContext *ctx)
{
    AppState *state = app_state_get();

    // get the brown height
    GRect bounds = layer_get_bounds(layer);
    float progress = (float)(state->brew_time_seconds - s_remaining_seconds) / state->brew_time_seconds;
    int brown_fill_height = (int)(bounds.size.h * progress);
    int start_fill_height = bounds.size.h - brown_fill_height; // y pos of the line between brown and white fill

    const char *timer_text = s_timer_buffer;
    GFont font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
    GRect text_bounds = GRect(0, bounds.size.h / 2 - 20, bounds.size.w, 40);

    // 🔹 Draw top portion (brown text on white)
    GRegion *top_region = gregion_create(GRect(0, 0, bounds.size.w, start_fill_height));
g   graphics_context_set_clip_region(ctx, top_region);
    graphics_context_set_text_color(ctx, GColorWindsorTan);
    // graphics_context_set_clip_rect(ctx, GRect(0, 0, bounds.size.w, start_fill_height));
    graphics_draw_text(ctx, timer_text, font, text_bounds,
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    gregion_destroy(top_region);

    // 🔹 Draw bottom portion (white text on brown)
    GRegion *bottom_region = gregion_create(GRect(0, start_fill_height, bounds.size.w, brown_fill_height));
    graphics_context_set_clip_region(ctx, bottom_region);
    graphics_context_set_text_color(ctx, GColorWhite);
    // graphics_context_set_clip_rect(ctx, GRect(0, start_fill_height, bounds.size.w, brown_fill_height));
    graphics_draw_text(ctx, timer_text, font, text_bounds,
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    gregion_destroy(bottom_region);

    // 🔄 Reset clip (good practice)
    graphics_context_set_clip_region(ctx, NULL);

    // draw brown fill for elapsed time
    graphics_context_set_fill_color(ctx, GColorWindsorTan);
    graphics_fill_rect(ctx,
                       GRect(0, bounds.size.h - brown_fill_height, bounds.size.w, brown_fill_height),
                       0, GCornerNone);

    // draw white fill for remaining time
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(
        ctx,
        GRect(0, 0, bounds.size.w, bounds.size.h - brown_fill_height),
        0,
        GCornerNone);
}
*/
/* Chat GPT not working
static void update_canvas_layer(Layer *layer, GContext *ctx)
{
    AppState *state = app_state_get();

    // get the brown height
    GRect bounds = layer_get_bounds(layer);
    float progress = (float)(state->brew_time_seconds - s_remaining_seconds) / state->brew_time_seconds;
    int brown_fill_height = (int)(bounds.size.h * progress);
    int start_fill_height = bounds.size.h - brown_fill_height; // y pos of the line between brown and white fill

    const char *timer_text = s_timer_buffer;
    GFont font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
    GRect text_bounds = GRect(0, bounds.size.h / 2 - 20, bounds.size.w, 40);

    // draw brown fill for elapsed time
    graphics_context_set_fill_color(ctx, GColorWindsorTan);
    graphics_fill_rect(ctx,
                       GRect(0, bounds.size.h - brown_fill_height, bounds.size.w, brown_fill_height),
                       0, GCornerNone);

    // draw white fill for remaining time
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(
        ctx,
        GRect(0, 0, bounds.size.w, bounds.size.h - brown_fill_height),
        0,
        GCornerNone);

    // 🔹 Draw top portion (brown text on white)
    graphics_context_set_text_color(ctx, GColorWindsorTan);
    graphics_draw_text(ctx, timer_text, font,
                       GRect(0, text_bounds.origin.y, bounds.size.w, start_fill_height - text_bounds.origin.y),
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    // 🔹 Draw bottom portion (white text on brown)
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, timer_text, font,
                       GRect(0, start_fill_height, bounds.size.w, bounds.size.h - start_fill_height),
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);


}
*/

static void update_canvas_layer(Layer *layer, GContext *ctx)
{
    AppState *state = app_state_get();

    GRect bounds = layer_get_bounds(layer);
    float progress = (float)(state->brew_time_seconds - s_remaining_seconds) / state->brew_time_seconds;
    int brown_fill_height = (int)(bounds.size.h * progress);
    int start_fill_height = bounds.size.h - brown_fill_height;

    // Draw rectangles
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, start_fill_height), 0, GCornerNone);

    graphics_context_set_fill_color(ctx, GColorWindsorTan);
    graphics_fill_rect(ctx, GRect(0, start_fill_height, bounds.size.w, brown_fill_height), 0, GCornerNone);

    /*
    // Draw text with shadow/outline effect
    const char *timer_text = s_timer_buffer;
    GFont font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
    GRect text_bounds = GRect(0, bounds.size.h / 2 - 30, bounds.size.w, 40);

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, GPoint(0, start_fill_height), GPoint(bounds.size.w, start_fill_height));

    // Draw black text on top
    if (start_fill_height > text_bounds.origin.y + text_bounds.size.h / 2)
    {
        // Draw white text as shadow (offset by 1px)
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, timer_text, font,
                           GRect(1, bounds.size.h / 2 - 29, bounds.size.w, 40),
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_context_set_text_color(ctx, GColorWindsorTan);
    }
    else
    {
        graphics_context_set_text_color(ctx, GColorWhite);
    }
    // graphics_context_set_text_color(ctx, GColorWindsorTan);
    graphics_draw_text(ctx, timer_text, font, text_bounds,
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    */
}

static void update_display(void *context)
{
    // Safety check: layers must exist
    if (!s_timer_layer || !s_instruction_layer || !s_canvas_layer)
    {
        return;
    }

    AppState *state = app_state_get();
    static char timer_text[32];

    if (s_timer_running && s_remaining_seconds > 0)
    {
        s_remaining_seconds--;
        s_timer = app_timer_register(1000, update_display, NULL); // Fire again in 1 second
        text_layer_set_text(s_instruction_layer, "Timer active\nSELECT to stop");
    }
    else if (s_timer_running && s_remaining_seconds == 0)
    {
        // Timer finished
        s_timer_running = false;
        vibes_double_pulse(); // Notify user
        text_layer_set_text(s_instruction_layer, "Brew Complete!\nSELECT to restart");
    }
    else
    {
        // Timer not running, show instructions
        s_remaining_seconds = state->brew_time_seconds;
        text_layer_set_text(s_instruction_layer, "SELECT to start\nUP/DOWN to adjust");
    }

    // get minutes and seconds for display
    s_display_min = s_remaining_seconds / 60;
    s_display_sec = s_remaining_seconds % 60;

    // layer_mark_dirty(s_canvas_layer);

    snprintf(timer_text, sizeof(timer_text), "Brew %01d:%02d", s_display_min, s_display_sec);
    snprintf(s_timer_buffer, sizeof(s_timer_buffer), "Brew %01d:%02d", s_display_min, s_display_sec);

    if (s_timer_running)
    {
        layer_mark_dirty(s_canvas_layer);
    }

    text_layer_set_text(s_timer_layer, timer_text);
}

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    window_set_background_color(window, GColorWindsorTan);

    s_header_container_layer = text_layer_create(GRect(0, 0, bounds.size.w, HEADER_HEIGHT));
    text_layer_set_background_color(s_header_container_layer, GColorBlack);
    int font_height = 30;

    // background layer
    s_canvas_layer = layer_create(GRect(0, HEADER_HEIGHT, bounds.size.w, bounds.size.h - HEADER_HEIGHT));
    layer_set_update_proc(s_canvas_layer, update_canvas_layer);
    layer_add_child(window_layer, s_canvas_layer);

    // fixed header layer
    s_header_layer = text_layer_create(GRect(0, HEADER_HEIGHT - font_height, bounds.size.w, font_height));
    text_layer_set_text(s_header_layer, "Press Timer");
    text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_background_color(s_header_layer, GColorBlack);
    text_layer_set_text_color(s_header_layer, GColorWhite);
    text_layer_set_overflow_mode(s_header_layer, GTextOverflowModeTrailingEllipsis);

    s_title_layer = text_layer_create(GRect(0, 10, bounds.size.w, 30));
    text_layer_set_text(s_title_layer, "Timer");
    text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
    text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    // layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

    s_timer_layer = text_layer_create(GRect(0, 40, bounds.size.w, 50));
    text_layer_set_text_alignment(s_timer_layer, GTextAlignmentCenter);
    text_layer_set_font(s_timer_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    layer_add_child(window_layer, text_layer_get_layer(s_timer_layer));

    s_instruction_layer = text_layer_create(GRect(0, 100, bounds.size.w, 50));
    text_layer_set_text(s_instruction_layer, "SELECT to restart");
    text_layer_set_text_alignment(s_instruction_layer, GTextAlignmentCenter);
    text_layer_set_font(s_instruction_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_layer, text_layer_get_layer(s_instruction_layer));

    layer_add_child(window_layer, text_layer_get_layer(s_header_container_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

    update_display(NULL);
}

static void window_unload(Window *window)
{
    // Cancel timer if it's running
    if (s_timer_running)
    {
        app_timer_cancel(s_timer);
        s_timer = NULL;
        s_timer_running = false;
    }

    text_layer_destroy(s_title_layer);
    text_layer_destroy(s_timer_layer);
    text_layer_destroy(s_instruction_layer);
    text_layer_destroy(s_header_layer);
    text_layer_destroy(s_header_container_layer);
    layer_destroy(s_canvas_layer);
    s_canvas_layer = NULL;
}

void timer_screen_push(void)
{
    AppState *state = app_state_get();
    state->brew_time_seconds = 15; // Default to 5 minutes
    s_remaining_seconds = state->brew_time_seconds;
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers){
                                             .load = window_load,
                                             .unload = window_unload,
                                         });
    window_stack_push(s_window, true);
}
