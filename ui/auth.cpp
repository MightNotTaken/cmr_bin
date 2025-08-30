#include "auth.hpp"
#include <string.h>
#include <database.h>

namespace Auth {
    void setup() {
        lv_obj_add_event_cb(ui_closeButton, [](lv_event_t * e) {
            if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                lv_textarea_set_text(ui_passwordInput, "");
                lv_scr_load(ui_homeScreen); 
            }
        }, LV_EVENT_ALL, NULL);

        lv_obj_add_event_cb(ui_Keyboard1, [](lv_event_t * e) {
            if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

            lv_obj_t * kb = lv_event_get_target(e);
            uint16_t btn_id = lv_btnmatrix_get_selected_btn(kb);
            const char * txt = lv_btnmatrix_get_btn_text(kb, btn_id);

            if (txt) {
                if (strlen(txt) == 3) {
                    if (txt[2] == 0x8c) {
                        String password = lv_textarea_get_text(ui_passwordInput);
                        password.trim(); // Remove extra spaces
                        Serial.printf("password: %s\n", password.c_str());
                        lv_textarea_set_text(ui_passwordInput, "");
                        Serial.printf("saved password: %s\n", getPassword().c_str());
                        if (getPassword().equals(password)) {
                            lv_scr_load(ui_editMaterialScreen);
                        } else {
                            lv_scr_load(ui_homeScreen);
                        }
                    }
                }

            }
        }, LV_EVENT_VALUE_CHANGED, NULL);
    }

    void setPassword(String password) {
        database.writeFile(password_file_ref, password);
    }
    
    String getPassword() {
        if (!database.hasFile(password_file_ref)) {
            database.writeFile(password_file_ref, "123456");
        }
        database.readFile(password_file_ref);
        
        return database.payload();
    }
};