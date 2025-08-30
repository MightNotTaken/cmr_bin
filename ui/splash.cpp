#include "splash.hpp"
void Splash::set_slider_value(void * obj, int32_t value) {
    lv_slider_set_value((lv_obj_t *)obj, value, LV_ANIM_OFF);
    if (value >= 100) {
        //chang_screen to ui_homeScreen
        lv_scr_load(ui_homeScreen);
    }
}


void Splash::splash() {
    lv_anim_t a;
    lv_obj_clear_flag(ui_slider, LV_OBJ_FLAG_CLICKABLE);
    lv_anim_init(&a);
    lv_anim_set_var(&a, ui_slider);
    lv_anim_set_exec_cb(&a, set_slider_value);
    lv_anim_set_time(&a, 5000); // Total duration = 5000 ms = 5 sec
    lv_anim_set_values(&a, 0, 100); // From 0 to 100
    lv_anim_start(&a);
}