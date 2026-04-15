#pragma once

#include <pebble.h>

typedef enum {
  APP_SCREEN_ROAST,
  APP_SCREEN_WATER,
  APP_SCREEN_TIMER
} AppScreen;

typedef enum {
  ROAST_LIGHT,
  ROAST_MEDIUM_LIGHT,
  ROAST_MEDIUM,
  ROAST_MEDIUM_DARK,
  ROAST_DARK
} RoastLevel;

typedef struct {
  AppScreen current_screen;
  RoastLevel roast_level;
  int water_oz;
  int coffee_grams;
  int brew_time_seconds;
} AppState;

void app_state_initialize(void);
AppState *app_state_get(void);
void app_state_set_screen(AppScreen screen);
AppScreen app_state_get_screen(void);