#include "RoastScreen.h"
#include "AppState.h"
#include "WaterScreen.h"
#include <pebble.h>
#include "UIContraints.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_header_layer;
static TextLayer *s_header_container_layer;

#define NUM_ROASTS 5
// #define HEADER_HEIGHT_RECT 35
#define HEADER_HEIGHT PBL_IF_ROUND_ELSE(40, HEADER_HEIGHT_RECT)
#define HEADER_PADDING PBL_IF_ROUND_ELSE(0, HEADER_HEIGHT_RECT)

// #define NUM_ROASTS 5
// #define HEADER_HEIGHT_RECT 35
// #define HEADER_HEIGHT PBL_IF_ROUND_ELSE(40, HEADER_HEIGHT_RECT)
// #define HEADER_PADDING PBL_IF_ROUND_ELSE(0, HEADER_HEIGHT_RECT)
// //#define MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT ((const int16_t) )
// #define MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT ((const int16_t) 68)
// #define MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT ((const int16_t) 24)
// #define MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT ((const int16_t) 84)
// #define MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT ((const int16_t) 32)
// #define MENU_CELL_BASIC_CELL_HEIGHT ((const int16_t) 44)

static const char *s_roast_names[NUM_ROASTS] = {
    "Light",
    "Medium Light",
    "Medium",
    "Medium Dark",
    "Dark"};

static uint16_t menu_get_num_sections(MenuLayer *menu_layer, void *data)
{
    return 1;
}

static uint16_t menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
    return NUM_ROASTS;
}

static int16_t menu_get_cell_height(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
    MenuIndex selected = menu_layer_get_selected_index(menu_layer);

    if (selected.section == cell_index->section && selected.row == cell_index->row)
    {
        return PBL_IF_ROUND_ELSE(MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT, MENU_CELL_BASIC_CELL_HEIGHT);
    }

    return PBL_IF_ROUND_ELSE(MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT, MENU_CELL_BASIC_CELL_HEIGHT);
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
    menu_cell_basic_draw(ctx, cell_layer, s_roast_names[cell_index->row], NULL, NULL);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
    AppState *state = app_state_get();
    state->roast_level = (RoastLevel)cell_index->row;
    app_state_set_screen(APP_SCREEN_WATER);
    water_screen_push();
}

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_header_container_layer = text_layer_create(GRect(0, 0, bounds.size.w, HEADER_HEIGHT));
    text_layer_set_background_color(s_header_container_layer, GColorBlack);
    int font_height = 30;

    // fixed header layer
    s_header_layer = text_layer_create(GRect(0, HEADER_HEIGHT - font_height, bounds.size.w, font_height));
    //   s_header_layer = text_layer_create(GRect(0, HEADER_HEIGHT - 28, bounds.size.w, 28);
    text_layer_set_text(s_header_layer, "Select Roast");
    text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_background_color(s_header_layer, GColorBlack);
    text_layer_set_text_color(s_header_layer, GColorWhite);
    text_layer_set_overflow_mode(s_header_layer, GTextOverflowModeTrailingEllipsis);

    GRect menu_bounds = GRect(0, HEADER_PADDING, bounds.size.w, bounds.size.h - HEADER_PADDING);
    s_menu_layer = menu_layer_create(menu_bounds);

    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
                                                     .get_num_sections = menu_get_num_sections,
                                                     .get_num_rows = menu_get_num_rows,
                                                     .get_cell_height = menu_get_cell_height,
                                                     .draw_row = menu_draw_row,
                                                     .select_click = menu_select_callback,
                                                 });

    menu_layer_set_normal_colors(s_menu_layer, GColorWhite, GColorBlack);
    menu_layer_set_highlight_colors(s_menu_layer, GColorWindsorTan, GColorWhite);
    menu_layer_set_click_config_onto_window(s_menu_layer, window);

    AppState *state = app_state_get();
    menu_layer_set_selected_index(s_menu_layer, (MenuIndex){.section = 0, .row = state->roast_level}, MenuRowAlignCenter, false);

    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_header_container_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_header_layer));
}

static void window_unload(Window *window)
{
    menu_layer_destroy(s_menu_layer);
    text_layer_destroy(s_header_layer);
    text_layer_destroy(s_header_container_layer);
    window_destroy(s_window);
    s_window = NULL;
}

void roast_screen_push(void)
{
    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers){
                                             .load = window_load,
                                             .unload = window_unload,
                                         });

    window_stack_push(s_window, true);
}