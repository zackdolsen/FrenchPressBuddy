#include "AppState.h"

static AppState s_state;

void app_state_initialize(void) {
  s_state.current_screen = APP_SCREEN_ROAST;
  s_state.roast_level = ROAST_MEDIUM;
  s_state.water_oz = 12;
  s_state.coffee_grams = 22.4;
  s_state.brew_time_seconds = 300;
}

AppState *app_state_get(void) {
  return &s_state;
}

void app_state_set_screen(AppScreen screen) {
  s_state.current_screen = screen;
}

AppScreen app_state_get_screen(void) {
  return s_state.current_screen;
}