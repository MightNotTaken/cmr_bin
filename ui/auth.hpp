#ifndef AUTH_H__
#define AUTH_H__
#include <ui.h>
#include <lvgl.h>
#include <Arduino.h>

#define password_file_ref     "/password.txt"

namespace Auth {
    void setup();
    void setPassword(String password);
    String getPassword();
};
#endif