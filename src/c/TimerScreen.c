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
static char s_timer_text[32] = "Brew for 0:00";
static int timer_y_pos = 0;
static int min_radius = 0;

#define HEADER_HEIGHT PBL_IF_ROUND_ELSE(40, HEADER_HEIGHT_RECT)
#define HEADER_PADDING PBL_IF_ROUND_ELSE(0, HEADER_HEIGHT_RECT)
#define SHAPEROUND PBL_IF_ROUND_ELSE(1, 0)
#define CENTER_Y_OFFSET 4 // 4 centers the text in the circle perfectly
#define DRAW_HEADER_OFFSET PBL_IF_ROUND_ELSE(HEADER_HEIGHT, HEADER_HEIGHT / 2 + CENTER_Y_OFFSET)
#define WATCH_TYPE get_watch_type()

// Forward declaration
static void update_display(void *context);

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
            window_stack_pop_all(true);
        }
        else
        {
            s_remaining_seconds = app_state_get()->brew_time_seconds;
            s_timer = app_timer_register(1000, update_display, NULL); // Fire every 1 second
            s_timer_running = true;
            layer_mark_dirty(s_canvas_layer);
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
    if (!s_timer_running && s_remaining_seconds != 0)
    {
        AppState *state = app_state_get();
        state->brew_time_seconds += 15; // Increase by 15 seconds
        update_display(NULL);
    }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    if (!s_timer_running && s_remaining_seconds != 0)
    {
        AppState *state = app_state_get();
        if (state->brew_time_seconds > 15)
        {
            state->brew_time_seconds -= 15; // Decrease by 15 seconds
            update_display(NULL);
        }
    }
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void update_canvas_layer(Layer *layer, GContext *ctx)
{
    Layer *window_layer = window_get_root_layer(s_window);
    GRect wbounds = layer_get_bounds(window_layer);

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

    // middle of layer
    int xPosMiddle = wbounds.size.w / 2;
    // int yPosMiddle = wbounds.size.h / 2 - HEADER_HEIGHT;
    int yPosMiddle = wbounds.size.h / 2 - DRAW_HEADER_OFFSET;

    int text_width = get_font_pixel_width(fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), s_timer_text);
    int text_height = get_font_pixel_height(fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), s_timer_text);

    // draw filler bubble behind text when timer is active
    if (s_timer_running)
    {
        // calculate radius of white bubble
        if (min_radius == 0 && brown_fill_height > 0)
        {
            min_radius = text_width / 2 + CENTER_Y_OFFSET;
            APP_LOG(APP_LOG_LEVEL_INFO, "Calculated min radius for circle: %d", min_radius);
        }
        else if (text_width / 2 + CENTER_Y_OFFSET > min_radius && brown_fill_height > 0)
        {
            min_radius = text_width / 2 + CENTER_Y_OFFSET;
            APP_LOG(APP_LOG_LEVEL_INFO, "Updated min radius for circle: %d", min_radius);
        }

        // for ROUND drawing
        if (SHAPEROUND)
        {
            graphics_context_set_fill_color(ctx, GColorWhite);
            graphics_fill_circle(ctx, GPoint(xPosMiddle, yPosMiddle), min_radius);
        }
        // for RECT drawing
        else
        {
            graphics_context_set_fill_color(ctx, GColorWhite);
            // GRect fill_bounds = GRect(xPosMiddle - text_width / 2 - CENTER_Y_OFFSET, yPosMiddle - text_height / 2 - CENTER_Y_OFFSET, text_width + CENTER_Y_OFFSET * 2, text_height + CENTER_Y_OFFSET * 2);
            GRect fill_bounds = GRect(xPosMiddle - min_radius, yPosMiddle - text_height / 2 - CENTER_Y_OFFSET, min_radius * 2, text_height + CENTER_Y_OFFSET * 2);
            graphics_fill_rect(ctx, fill_bounds, CENTER_Y_OFFSET, GCornersAll);
        }
    }

    // draw timer text
    APP_LOG(APP_LOG_LEVEL_INFO, "Calculated text width: %d, height: %d for string %s", text_width, text_height, s_timer_text);

    GRect text_bounds = GRect(xPosMiddle - text_width / 2, yPosMiddle - text_height / 2 - CENTER_Y_OFFSET, text_width, text_height);

    // Draw text
    if (!s_timer_running && s_remaining_seconds == 0)
    {
        graphics_context_set_text_color(ctx, GColorWhite);
    }
    else
    {
        graphics_context_set_text_color(ctx, GColorWindsorTan);
    }
    graphics_draw_text(ctx, s_timer_text, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), text_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    // grid for layout testing
    // graphics_context_set_fill_color(ctx, GColorBlack);
    // graphics_fill_rect(ctx, GRect(0, yPosMiddle, bounds.size.w, 1), 0, GCornerNone);
}

static void update_display(void *context)
{
    // Safety check: layers must exist
    if (!s_timer_layer || !s_instruction_layer || !s_canvas_layer)
    {
        return;
    }

    AppState *state = app_state_get();

    if (s_timer_running && s_remaining_seconds > 0)
    {
        s_remaining_seconds--;
        // get minutes and seconds for display
        s_display_min = s_remaining_seconds / 60;
        s_display_sec = s_remaining_seconds % 60;
        snprintf(s_timer_text, sizeof(s_timer_text), "%01d:%02d", s_display_min, s_display_sec);
        s_timer = app_timer_register(1000, update_display, NULL); // Fire again in 1 second
        text_layer_set_text(s_instruction_layer, "");
        // if(PBL_IF_COLOR_ELSE(1,0))
        // {
        //     text_layer_set_text(s_instruction_layer, "Timer active\nSELECT to stop");
        // }
        // else
        // {
        //     text_layer_set_text(s_instruction_layer, "");
        // }
    }
    else if (s_timer_running && s_remaining_seconds == 0)
    {
        // Timer finished
        s_timer_running = false;
        vibes_double_pulse(); // Notify user
        text_layer_set_text(s_instruction_layer, "SELECT to close app");
        text_layer_set_text_color(s_timer_layer, GColorWhite);
        if (WATCH_TYPE == SCREEN_TYPE_OG_RECT || WATCH_TYPE == SCREEN_TYPE_OG_ROUND)
        {

            snprintf(s_timer_text, sizeof(s_timer_text), "Brewing Done!");
        }
        else
        {
            snprintf(s_timer_text, sizeof(s_timer_text), "Brew Complete!");
        }

        layer_mark_dirty(s_canvas_layer);
    }
    else
    {
        // Timer not running, show instructions
        s_remaining_seconds = state->brew_time_seconds;
        // get minutes and seconds for display
        s_display_min = s_remaining_seconds / 60;
        s_display_sec = s_remaining_seconds % 60;
        // text_layer_set_text(s_instruction_layer, "SELECT to start\nUP/DOWN to adjust");
        text_layer_set_text(s_instruction_layer, "");

        if (WATCH_TYPE == SCREEN_TYPE_RECT_V2)
        {
            snprintf(s_timer_text, sizeof(s_timer_text), "Brew for\n%01d:%02d", s_display_min, s_display_sec);
        }
        else
        {
            snprintf(s_timer_text, sizeof(s_timer_text), "Brew for %01d:%02d", s_display_min, s_display_sec);
        }
        // snprintf(s_timer_text, sizeof(s_timer_text), "Brew for\n%01d:%02d", s_display_min, s_display_sec);
    }

    // layer_mark_dirty(s_canvas_layer);

    // snprintf(s_timer_text, sizeof(s_timer_text), "Brew %01d:%02d", s_display_min, s_display_sec);

    if (s_timer_running)
    {
        layer_mark_dirty(s_canvas_layer);
    }

    // text_layer_set_text(s_timer_layer, s_timer_text);
}

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    APP_LOG(APP_LOG_LEVEL_INFO, "Calculated middle BASE y as %d", bounds.size.h / 2);
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
    text_layer_set_text(s_header_layer, "Brew Timer");
    text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_background_color(s_header_layer, GColorBlack);
    text_layer_set_text_color(s_header_layer, GColorWhite);
    text_layer_set_overflow_mode(s_header_layer, GTextOverflowModeTrailingEllipsis);

    int timer_font_height = get_font_pixel_height(fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), s_timer_text);
    timer_y_pos = bounds.size.h / 2 - timer_font_height / 2 - CENTER_Y_OFFSET;
    APP_LOG(APP_LOG_LEVEL_INFO, "Draw height for box %dpx, medium =  %dpx", timer_y_pos, bounds.size.h / 2);
    s_timer_layer = text_layer_create(GRect(0, timer_y_pos, bounds.size.w, timer_font_height * 2));
    text_layer_set_text_alignment(s_timer_layer, GTextAlignmentCenter);
    text_layer_set_font(s_timer_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_background_color(s_timer_layer, GColorClear);
    text_layer_set_text_color(s_timer_layer, GColorWindsorTan);
    layer_add_child(window_layer, text_layer_get_layer(s_timer_layer));

    // bottom instruction layer
    if (WATCH_TYPE == SCREEN_TYPE_OG_RECT)
    {
        s_instruction_layer = text_layer_create(GRect(0, bounds.size.h - 30, bounds.size.w, 50));
    }
    else
    {
        s_instruction_layer = text_layer_create(GRect(0, bounds.size.h / 4 * 3, bounds.size.w, 50));
    }
    // s_instruction_layer = text_layer_create(GRect(0, bounds.size.h / 4 * 3, bounds.size.w, 50));
    text_layer_set_text(s_instruction_layer, "SELECT to restart");
    text_layer_set_text_alignment(s_instruction_layer, GTextAlignmentCenter);
    text_layer_set_font(s_instruction_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_background_color(s_instruction_layer, GColorClear);
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

    window_destroy(s_window);
    s_window = NULL;
}

void timer_screen_push(void)
{
    AppState *state = app_state_get();
    state->brew_time_seconds = 300; // Default to 300 sec
    s_remaining_seconds = state->brew_time_seconds;
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers){
                                             .load = window_load,
                                             .unload = window_unload,
                                         });
    window_stack_push(s_window, true);
}
