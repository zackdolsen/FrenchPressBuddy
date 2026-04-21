#include <pebble.h>
#include "AppState.h"
#include "RoastScreen.h"
#include "WaterScreen.h"
#include "TimerScreen.h"
#include "UIContraints.h"

static void push_current_screen(void) {
  switch (app_state_get_screen()) {
    case APP_SCREEN_ROAST:
      roast_screen_push();
      break;
    case APP_SCREEN_WATER:
      water_screen_push();
      break;
    case APP_SCREEN_TIMER:
      timer_screen_push();
      break;
  }
}

static void init(void) {
  app_state_initialize();
  watch_type_init();

  // Start the app at the roast selection screen.
  // To test a single screen directly, replace the current screen below or call one of:
  //   water_screen_push();
  //   timer_screen_push();
  push_current_screen();
}

static void deinit(void) {
  // Individual screens own and destroy their own windows.
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

