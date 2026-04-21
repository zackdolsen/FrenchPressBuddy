#pragma once

#define HEADER_HEIGHT_RECT 35
// #define MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT ((const int16_t) )
#define MENU_ROUND_FOCUSED_SHORT_CELL_HEIGHT ((const int16_t)56) //selected height for round 1
#define MENU_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT ((const int16_t)28) //unselected height for round 1
#define MENU_ROUND_FOCUSED_TALL_CELL_HEIGHT ((const int16_t)68) //selected height for round 2 (used to be 68/84)
#define MENU_ROUND_UNFOCUSED_TALL_CELL_HEIGHT ((const int16_t)34) //unselected height for round 2 (24/32)
#define MENU_BASIC_CELL_HEIGHT ((const int16_t)44) //height for all cells on rectangular watches

typedef enum {
  SCREEN_TYPE_OG_RECT,
  SCREEN_TYPE_OG_ROUND,
  SCREEN_TYPE_RECT_V2,
  SCREEN_TYPE_ROUND_V2
} ScreenType;

void watch_type_init(void);
ScreenType get_watch_type(void);

static inline int get_font_pixel_height(GFont font, const char *test_str)
{

  // const char *test_str = "Ag";

  // Create a large bounds box so text isn't constrained
  GRect max_bounds = GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT);

  // Measure text layout
  GSize size = graphics_text_layout_get_content_size(
      test_str,
      font,
      max_bounds,
      GTextOverflowModeWordWrap,
      GTextAlignmentLeft);

  APP_LOG(APP_LOG_LEVEL_INFO, "Font height: %d", size.h);

  return size.h;
}

static inline int get_font_pixel_width(GFont font, const char *test_str)
{

  // const char *test_str = "Ag";

  // Create a large bounds box so text isn't constrained
  GRect max_bounds = GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT);

  // Measure text layout
  GSize size = graphics_text_layout_get_content_size(
      test_str,
      font,
      max_bounds,
      GTextOverflowModeWordWrap,
      GTextAlignmentLeft);

  APP_LOG(APP_LOG_LEVEL_INFO, "Font width: %d px for string: %s", size.h, test_str);

  return size.w;
}
