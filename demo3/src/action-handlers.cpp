#include <lvgl.h>
#include <TFT_eSPI.h>
#include "ui/ui.h"
#include "ui/actions.h"

extern "C" void action_navigate_main(lv_event_t *e) {
    loadScreen(SCREEN_ID_MAIN);
}

extern "C" void action_navigate_screen_1(lv_event_t *e) {
    // loadScreen(SCREEN_ID_SCREEN1);
}

extern "C" void action_navigate_screen_2(lv_event_t *e) {
    // loadScreen(SCREEN_ID_SCREEN2);
}

