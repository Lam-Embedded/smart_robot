#include "screenTestLabel.h"
#include "Fonts/fonts.h"

lv_obj_t* screenTestLabel() {
    /* Create a screen */
    static lv_obj_t *screenNomal = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screenNomal, lv_color_make(255, 0, 0), LV_PART_MAIN);

    /* Create label for button */
    lv_obj_t* labelNomal = lv_label_create(screenNomal);
    lv_label_set_text(labelNomal, "Nga ngố hôm nay em ăn gì");
    lv_obj_align(labelNomal, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(labelNomal, 300);
    lv_obj_set_style_text_font(labelNomal, &Nunito_Bold_24, LV_PART_MAIN);
    lv_label_set_long_mode(labelNomal, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);

    return screenNomal;
}