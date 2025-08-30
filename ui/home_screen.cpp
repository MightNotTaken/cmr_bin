#include "home_screen.hpp"
#include "sensor-configuration.h"
#include "furnace-ctrl.hpp"

namespace HomeScreen {
  void setup() {
    lv_obj_add_event_cb(ui_materialEdit, [](lv_event_t * e) {
      if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
          lv_scr_load(ui_authentication);      
      }
    }, LV_EVENT_ALL, NULL);
    
    lv_obj_add_event_cb(ui_prev, [](lv_event_t * e) {
      if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
          FurnaceCtrl::activatePrevFrunace();      
      }
    }, LV_EVENT_ALL, NULL);
    
    lv_obj_add_event_cb(ui_next, [](lv_event_t * e) {
      if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
          FurnaceCtrl::activateNextFrunace();
      }
    }, LV_EVENT_ALL, NULL);
  }

  void setSignalStrength(int signal) {
      lv_obj_t * network_sticks[] = {
          ui_below20,
          ui_below40,
          ui_below60,
          ui_below80,
          ui_fullNetwork
      };

      for (int i = 0; i < 5; i++) {
          if (signal > 20 * i) {
              lv_obj_set_style_opa(network_sticks[i], LV_OPA_COVER, LV_PART_MAIN); // fully visible
          } else {
              lv_obj_set_style_opa(network_sticks[i], LV_OPA_30, LV_PART_MAIN); // faded
          }
      }
  }

  void setTime(String time) {
    lv_label_set_text(ui_timeText, time.c_str());
  }

  
  void displayFurnace(String name, String stats, String mac, int level, int state, String material) {
    if (level > 30000) {
      level = 0;
    }
    lv_label_set_text(ui_furnaceName, (name.length() ? name : mac).c_str());
    lv_label_set_text(ui_levelValue, String(level).c_str());

    lv_obj_set_style_opa(ui_audioIndicator, state & SOUND_ON ? LV_OPA_COVER : LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_opa(ui_redIndicator, state & RED_ON ? LV_OPA_COVER : LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_opa(ui_greenIndicator, state & GREEN_ON ? LV_OPA_COVER : LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_opa(ui_yellowIndicator, state & YELLOW_ON ? LV_OPA_COVER : LV_OPA_30, LV_PART_MAIN);

    lv_label_set_text(ui_stats, stats.c_str());

    lv_dropdown_set_options(ui_materialOptions, material.c_str());
  }

  
}