SET(SOURCES ui_splash.c
    ui_homeScreen.c
    ui_authentication.c
    ui_editMaterialScreen.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_logo_png.c
    ui_img_small_logo_png.c
    ui_img_audio_png.c
    ui_img_edit_png.c)

add_library(ui ${SOURCES})
