#include <Arduino.h>
#include <lvgl.h>
#include <TFT_Touch.h>
#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#endif
#include "screen/screenTest.h"
#include "screen/screenTestLabel.h"

/* -------------------------------------------------------------------------- */
/*                          Display & Touch Settings                          */
/* -------------------------------------------------------------------------- */
#define TFT_HOR_RES   320
#define TFT_VER_RES   240
#define TFT_ROTATION  LV_DISPLAY_ROTATION_90

#define HMIN 534
#define HMAX 3496
#define VMIN 635
#define VMAX 3395
#define XYSWAP 1 // 0 or 1 

#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

SPIClass touchscreenSpi = SPIClass(HSPI);
//TFT_Touch touch(XPT2046_CS, XPT2046_CLK, XPT2046_MOSI, XPT2046_MISO);
XPT2046_Touchscreen touchScreen(XPT2046_CS, XPT2046_IRQ);

#if LV_USE_LOG
static void my_print(lv_log_level_t level, const char* msg) {
  (void)level;
  Serial.println(msg);
}
#endif

/* Forward declarations */
static void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map);
static void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data);
static uint32_t my_tick();
void activity_1();

/* -------------------------------------------------------------------------- */
/*                                    Setup                                   */
/* -------------------------------------------------------------------------- */

void setup() {
  Serial.begin(115200);
  touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchScreen.begin(touchscreenSpi);

  Serial.printf("LVGL %d.%d.%d\n", lv_version_major(), lv_version_minor(), lv_version_patch());

  lv_init();
  lv_tick_set_cb(my_tick);

#if LV_USE_LOG
  lv_log_register_print_cb(my_print);
#endif
  /* Display */
  lv_display_t* disp;
#if LV_USE_TFT_ESPI
  disp = lv_tft_espi_create(TFT_VER_RES, TFT_HOR_RES, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, TFT_ROTATION);
#else
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, nullptr, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

  /* Touch input */
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  /* button */
  activity_1();

}

/* -------------------------------------------------------------------------- */
/*                                     Loop                                   */
/* -------------------------------------------------------------------------- */
void loop() {
  lv_timer_handler();
  delay(5);
}

/* -------------------------------------------------------------------------- */
/*                            LVGL HAL Callbacks                              */
/* -------------------------------------------------------------------------- */
static void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
  LV_UNUSED(disp);
  LV_UNUSED(area);
  LV_UNUSED(px_map);
  lv_display_flush_ready(disp);
}

static void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data) {
  (void)indev;
  if (!touchScreen.touched()) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  data->state   = LV_INDEV_STATE_PRESSED;
  data->point.x = map(touchScreen.getPoint().x, 350, 3800, 1, TFT_VER_RES);
  data->point.y = map(touchScreen.getPoint().y, 500, 3800, 1, TFT_HOR_RES);
  
  // Serial.printf("x: %d - y: %d \n", data->point.x, data->point.y);
}

static uint32_t my_tick() {
  return millis();
}

void activity_1() {
  /* screen */
  static lv_obj_t *screen = screenTestLabel();
  lv_screen_load(screen);
}
