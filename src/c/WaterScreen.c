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
static TextLayer *s_bottom_container_layer;
static TextLayer *s_water_header_layer;
static TextLayer *s_water_value_layer;
static TextLayer *s_coffee_header_layer;
static TextLayer *s_coffee_value_layer;

// images
static BitmapLayer *s_water_image_layer;
static GBitmap *s_water_image;
static BitmapLayer *s_coffee_image_layer;
static GBitmap *s_coffee_image;
static GRect s_water_image_target;
static GRect s_coffee_image_target;

// Color
static GColor s_value_text_color = GColorWhite;
static int move_direction; // 0 for up, 1 for down, 0 for right wiggle, 1 for left wiggle

#define waterOzingrams 28.3495
#define HEADER_HEIGHT PBL_IF_ROUND_ELSE(40, HEADER_HEIGHT_RECT)
#define HEADER_PADDING PBL_IF_ROUND_ELSE(0, HEADER_HEIGHT_RECT)
#define CENTER_V_OFFSET 18
#define CENTER_Y_OFFSET 2 // text offset to center 
#define ANIMATION_DURATION 150
#define ANIMATION_DISTANCE 10
#define WATCH_TYPE get_watch_type()

// Forward declaration
static void update_display(void);

// animation

static void anim_stopped(Animation *anim, bool finished, void *context)
{
    animation_destroy(anim);
}

static void animate_layer(Layer *layer, GRect target, int duration)
{
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, NULL, &target);

    animation_set_duration((Animation *)anim, duration);
    animation_set_curve((Animation *)anim, AnimationCurveEaseOut);

    animation_set_handlers((Animation *)anim, (AnimationHandlers){.stopped = anim_stopped}, NULL);

    animation_schedule((Animation *)anim);
}

static void wiggle_back()
{
    animate_layer(bitmap_layer_get_layer(s_coffee_image_layer), s_coffee_image_target, ANIMATION_DURATION / 2);
}

static void wiggle2()
{
    GRect coffee_up = s_coffee_image_target;

    if (move_direction == 1)
    {
        coffee_up.origin.x += ANIMATION_DISTANCE / 3;
    }
    else
    {
        coffee_up.origin.x -= ANIMATION_DISTANCE / 3;
    }

    animate_layer(bitmap_layer_get_layer(s_coffee_image_layer), coffee_up, ANIMATION_DURATION);

    app_timer_register(ANIMATION_DURATION, wiggle_back, NULL);
}

static void wiggle()
{
    GRect coffee_up = s_coffee_image_target;
    if (move_direction == 1)
    {
        coffee_up.origin.x -= ANIMATION_DISTANCE / 3;
    }
    else
    {
        coffee_up.origin.x += ANIMATION_DISTANCE / 3;
    }

    animate_layer(bitmap_layer_get_layer(s_coffee_image_layer), coffee_up, ANIMATION_DURATION / 2);

    app_timer_register(ANIMATION_DURATION / 2, wiggle2, NULL);

    // psleep(ANIMATION_DURATION / 2);

    // if (down == 1)
    // {
    //     coffee_up.origin.x += ANIMATION_DISTANCE;
    // }
    // else
    // {
    //     coffee_up.origin.x -= ANIMATION_DISTANCE;
    // }

    // animate_layer(bitmap_layer_get_layer(s_coffee_image_layer), coffee_up, ANIMATION_DURATION);

    // psleep(ANIMATION_DURATION);
    // animate_layer(bitmap_layer_get_layer(s_coffee_image_layer), s_coffee_image_target, ANIMATION_DURATION / 2);
}

static void bounce_back()
{
    animate_layer(bitmap_layer_get_layer(s_water_image_layer), s_water_image_target, ANIMATION_DURATION);
}

static void bounce()
{
    GRect water_up = s_water_image_target;

    if (move_direction == 1)
    {
        water_up.origin.y += ANIMATION_DISTANCE;
    }
    else
    {
        water_up.origin.y -= ANIMATION_DISTANCE;
    }

    animate_layer(bitmap_layer_get_layer(s_water_image_layer), water_up, ANIMATION_DURATION);

    app_timer_register(ANIMATION_DURATION, bounce_back, NULL);
}

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
    move_direction = 0;
    bounce();
    wiggle();
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
        move_direction = 1;
        bounce();
        wiggle();
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

// add in for if i want to add in a background to text bubbles
//  static void layer_update_proc(Layer *layer, GContext *ctx)
//  {

// }

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    window_set_background_color(window, GColorWindsorTan);

    s_header_container_layer = text_layer_create(GRect(0, 0, bounds.size.w, HEADER_HEIGHT));
    text_layer_set_background_color(s_header_container_layer, GColorBlack);
    int font_height = 30;

    // image loading
    s_water_image = gbitmap_create_with_resource(RESOURCE_ID_WATER_COLOR_ICON);
    GRect water_image_bounds = gbitmap_get_bounds(s_water_image);
    s_coffee_image = gbitmap_create_with_resource(RESOURCE_ID_COFFEE_WHITE_ICON);
    GRect coffee_image_bounds = gbitmap_get_bounds(s_coffee_image);

    // fixed header layer
    s_header_layer = text_layer_create(GRect(0, HEADER_HEIGHT - font_height, bounds.size.w, font_height));
    text_layer_set_text(s_header_layer, "Brew Amounts");
    text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_background_color(s_header_layer, GColorBlack);
    text_layer_set_text_color(s_header_layer, GColorWhite);
    text_layer_set_overflow_mode(s_header_layer, GTextOverflowModeTrailingEllipsis);

    // water header layer
    s_water_header_layer = text_layer_create(GRect(0, bounds.size.h / 2 - font_height / 2 - CENTER_V_OFFSET  - CENTER_Y_OFFSET - water_image_bounds.size.h / 2, bounds.size.w / 2, font_height));
    text_layer_set_text(s_water_header_layer, "Water");
    text_layer_set_text_alignment(s_water_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_water_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_water_header_layer, s_value_text_color);
    text_layer_set_background_color(s_water_header_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_water_header_layer));

    // coffee header layer
    s_coffee_header_layer = text_layer_create(GRect(bounds.size.w / 2, bounds.size.h / 2 - font_height / 2 - CENTER_V_OFFSET - CENTER_Y_OFFSET - coffee_image_bounds.size.h / 2, bounds.size.w / 2, font_height));
    text_layer_set_text(s_coffee_header_layer, "Coffee");
    text_layer_set_text_alignment(s_coffee_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_coffee_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_coffee_header_layer, s_value_text_color);
    text_layer_set_background_color(s_coffee_header_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_coffee_header_layer));

    // water value layer
    s_water_value_layer = text_layer_create(GRect(0, bounds.size.h / 2 - font_height / 2 + CENTER_V_OFFSET - CENTER_Y_OFFSET+ water_image_bounds.size.h / 2, bounds.size.w / 2, font_height));
    text_layer_set_text_alignment(s_water_value_layer, GTextAlignmentCenter);
    text_layer_set_font(s_water_value_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_water_value_layer, s_value_text_color);
    text_layer_set_background_color(s_water_value_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_water_value_layer));

    // coffee value layer
    s_coffee_value_layer = text_layer_create(GRect(bounds.size.w / 2, bounds.size.h / 2 - font_height / 2 + CENTER_V_OFFSET - CENTER_Y_OFFSET + coffee_image_bounds.size.h / 2 , bounds.size.w / 2, font_height));
    text_layer_set_text_alignment(s_coffee_value_layer, GTextAlignmentCenter);
    text_layer_set_font(s_coffee_value_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_color(s_coffee_value_layer, s_value_text_color);
    text_layer_set_background_color(s_coffee_value_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_coffee_value_layer));

    // water image layer
    s_water_image_target = GRect(0, bounds.size.h / 2 - water_image_bounds.size.h / 2, bounds.size.w / 2, water_image_bounds.size.h);
    s_water_image_layer = bitmap_layer_create(s_water_image_target);
    bitmap_layer_set_bitmap(s_water_image_layer, s_water_image);
    bitmap_layer_set_compositing_mode(s_water_image_layer, GCompOpSet);
    bitmap_layer_set_alignment(s_water_image_layer, GAlignCenter);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_water_image_layer));

    // coffee image layer
    s_coffee_image_target = GRect(bounds.size.w / 2, bounds.size.h / 2 - coffee_image_bounds.size.h / 2, bounds.size.w / 2, coffee_image_bounds.size.h);
    s_coffee_image_layer = bitmap_layer_create(s_coffee_image_target);
    bitmap_layer_set_bitmap(s_coffee_image_layer, s_coffee_image);
    bitmap_layer_set_compositing_mode(s_coffee_image_layer, GCompOpSet);
    bitmap_layer_set_alignment(s_coffee_image_layer, GAlignCenter);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_coffee_image_layer));

    int bottom_text_start_height;
    int bottom_header_font_height = get_font_pixel_height(fonts_get_system_font(FONT_KEY_GOTHIC_24), "Based on Roast Level");

    // bottom container layer for background of bottom header
    if (WATCH_TYPE == SCREEN_TYPE_OG_RECT && PBL_IF_BW_ELSE(1, 0))
    {
        s_bottom_container_layer = text_layer_create(GRect(0, bounds.size.h - bottom_header_font_height, bounds.size.w, bottom_header_font_height + 10));
        text_layer_set_background_color(s_bottom_container_layer, GColorBlack);
        layer_add_child(window_layer, text_layer_get_layer(s_bottom_container_layer));
    }

    // bottom header
    if (WATCH_TYPE == SCREEN_TYPE_RECT_V2 || WATCH_TYPE == SCREEN_TYPE_ROUND_V2) //newer larger watches
    {
        bottom_text_start_height = bounds.size.h / 2 + font_height + CENTER_V_OFFSET + coffee_image_bounds.size.h / 2;
    }
    else if (WATCH_TYPE == SCREEN_TYPE_OG_ROUND) // OG round
    {
        bottom_text_start_height = bounds.size.h - HEADER_HEIGHT;
    }
     else //OG rect
    {
        bottom_text_start_height = bounds.size.h - bottom_header_font_height - CENTER_V_OFFSET/4;
    }

    s_bottom_header_layer = text_layer_create(GRect(0, bottom_text_start_height, bounds.size.w, HEADER_HEIGHT));
    if (WATCH_TYPE == SCREEN_TYPE_OG_ROUND)
    {
        text_layer_set_text(s_bottom_header_layer, "Based on Roast");
    }
    else
    {
        text_layer_set_text(s_bottom_header_layer, "Based on Roast Level");
    }
    text_layer_set_text_alignment(s_bottom_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_bottom_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_background_color(s_bottom_header_layer, GColorClear);
    // text_layer_set_background_color(s_bottom_header_layer, GColorClear);
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
    text_layer_destroy(s_water_header_layer);
    text_layer_destroy(s_coffee_header_layer);
    text_layer_destroy(s_water_value_layer);
    text_layer_destroy(s_coffee_value_layer);
    text_layer_destroy(s_bottom_header_layer);
    text_layer_destroy(s_bottom_container_layer);

    // if (s_water_image_animation)
    // {
    //     animation_destroy((Animation *)s_water_image_animation);
    // }
    // if (s_coffee_image_animation)
    // {
    //     animation_destroy((Animation *)s_coffee_image_animation);
    // }

    bitmap_layer_destroy(s_water_image_layer);
    gbitmap_destroy(s_water_image);

    bitmap_layer_destroy(s_coffee_image_layer);
    gbitmap_destroy(s_coffee_image);

    window_destroy(s_window);
    s_window = NULL;
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
