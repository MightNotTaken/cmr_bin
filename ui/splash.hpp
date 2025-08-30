#ifndef SPLASH_H__
#define SPLASH_H__
#include <Arduino.h>
#include <ui.h>
namespace Splash {
    void splash();
    void set_slider_value(void * obj, int32_t value);
    void slider_completes(lv_anim_t * a);
};
#endif