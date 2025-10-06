#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *screen1;
    lv_obj_t *screen2;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SCREEN1 = 2,
    SCREEN_ID_SCREEN2 = 3,
};

void create_screen_main();
void tick_screen_main();

void create_screen_screen1();
void tick_screen_screen1();

void create_screen_screen2();
void tick_screen_screen2();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/