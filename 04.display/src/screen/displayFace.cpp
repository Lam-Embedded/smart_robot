#include "displayFace.h"

lv_obj_t* displayFace(){
    /* Create a screen */
    static lv_obj_t *screenNomal = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screenNomal, lv_color_make(255, 0, 0), LV_PART_MAIN);


    lv_obj_t * img;
    img = lv_image_create(screenNomal);
    lv_image_set_src(img, &frame_00035);
    lv_obj_center(img);

    return screenNomal;
}
