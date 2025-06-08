#include "screenMove.h"
#include "Fonts/fonts.h"
#include <Arduino.h>

lv_obj_t* screenMove() {
    /* Create a screen */
    static lv_obj_t *screenNomal = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screenNomal, lv_color_make(255, 0, 0), LV_PART_MAIN);

    // Tạo đối tượng mới với kích thước 32x64
    lv_obj_t *myObject = lv_obj_create(screenNomal);                                  // Tạo đối tượng con của screen
    lv_obj_set_size(myObject, 32, 64);                                           // Đặt kích thước 32x64 pixels
    lv_obj_align(myObject, LV_ALIGN_CENTER, 0, 0);                             // Đặt đối tượng ở chính giữa màn hình
    lv_obj_set_style_bg_color(myObject, lv_color_make(0, 255, 0), LV_PART_MAIN); // Đặt màu nền cho đối tượng (ví dụ: màu xanh)

    // Thêm sự kiện nhấn giữ (long press)
    lv_obj_add_event(
        myObject,
        [](lv_event_t *e) {
            lv_event_code_t code = lv_event_get_code(e);
            lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
            lv_point_t point;
            lv_indev_get_vect(lv_indev_active(), &point);

            lv_obj_set_pos(obj, lv_obj_get_x(obj) + point.x, lv_obj_get_y(obj) + point.y);
            Serial.printf("x:%d - y:%d\n", point.x, point.y);
        },
        LV_EVENT_PRESSING, // Sử dụng LV_EVENT_ALL để bắt tất cả các sự kiện
        nullptr
    );

    return screenNomal;
}   