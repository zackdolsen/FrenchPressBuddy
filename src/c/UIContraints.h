#pragma once

#define HEADER_HEIGHT_RECT 35
//#define MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT ((const int16_t) )
#define MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT ((const int16_t) 68)
#define MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT ((const int16_t) 24)
#define MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT ((const int16_t) 84)
#define MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT ((const int16_t) 32)
#define MENU_CELL_BASIC_CELL_HEIGHT ((const int16_t) 44)

static inline int get_font_pixel_height(GFont font, const char *test_str) {

    // const char *test_str = "Ag";
 
  // Create a large bounds box so text isn't constrained
  GRect max_bounds = GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT);

  // Measure text layout
  GSize size = graphics_text_layout_get_content_size(
    test_str,
    font,
    max_bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft
  );

  APP_LOG(APP_LOG_LEVEL_INFO, "Font height: %d", size.h);

  return size.h;
}

static inline int get_font_pixel_width(GFont font, const char *test_str) {

    // const char *test_str = "Ag";
 
  // Create a large bounds box so text isn't constrained
  GRect max_bounds = GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT);

  // Measure text layout
  GSize size = graphics_text_layout_get_content_size(
    test_str,
    font,
    max_bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft
  );

  APP_LOG(APP_LOG_LEVEL_INFO, "Font width: %d px for string: %s", size.h, test_str);

  return size.w;
}
