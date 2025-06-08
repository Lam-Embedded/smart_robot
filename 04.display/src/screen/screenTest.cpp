#include "screenTestLabel.h"

lv_obj_t* screenTest() {
    /* Create a screen */
    static lv_obj_t *screenNomal = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screenNomal, lv_color_make(255, 0, 0), LV_PART_MAIN);

    /* Create a button in screen */
    static lv_obj_t* buttonNomal = lv_button_create(screenNomal);
    // Kich thuoc cua nut nhan
    lv_obj_set_size(buttonNomal, 100, 50);
    // Vi tri cua nut nhan
    lv_obj_align(buttonNomal, LV_ALIGN_CENTER, 0, 0);
    // Mau nut nhan
    lv_obj_set_style_bg_color(buttonNomal, lv_color_make(22, 88, 44), LV_PART_MAIN);

    /* Create label for button */
    lv_obj_t* labelNomal = lv_label_create(buttonNomal);
    lv_label_set_text(labelNomal, "Button");
    lv_obj_align(labelNomal, LV_ALIGN_CENTER, 0, 0);

    /* Thiet lap su kien khi an nut nhan */
    lv_obj_add_flag(buttonNomal, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_event(
        buttonNomal, 
        [](lv_event_t* e){
            if (lv_obj_has_state(lv_event_get_current_target_obj(e), LV_STATE_CHECKED)) {
                // analogWrite(TFT_BL, 255);
                lv_obj_set_style_bg_color(buttonNomal, lv_color_make(22, 88, 44), LV_PART_SCROLLBAR);
                Serial.println("BUTTON is Check \n");
            }
            else {
                lv_obj_set_style_bg_color(buttonNomal, lv_color_make(0, 200, 44), LV_PART_MAIN);
                Serial.println("BUTTON is NOT Check \n");
                // analogWrite(TFT_BL, 4);
            }
        }, 
        LV_EVENT_VALUE_CHANGED, nullptr);

    return screenNomal;
}