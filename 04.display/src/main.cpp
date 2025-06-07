#include <Arduino.h>
#include <lvgl.h>
#include <TFT_Touch.h>
#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
#endif

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
#define XYSWAP 0 // 0 or 1 

#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

TFT_Touch touch(XPT2046_CS, XPT2046_CLK, XPT2046_MOSI, XPT2046_MISO);

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

/* -------------------------------------------------------------------------- */
/*                                    Setup                                   */
/* -------------------------------------------------------------------------- */
void setup() {
  Serial.begin(115200);
  Serial.printf("LVGL %d.%d.%d\n", lv_version_major(), lv_version_minor(), lv_version_patch());

  touch.setCal(HMIN, HMAX, VMIN, VMAX, TFT_VER_RES, TFT_HOR_RES, XYSWAP);
  touch.setRotation(1);

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

  // /* Button UI*/
  // lv_obj_t* button = lv_button_create(lv_screen_active());
  // lv_obj_align(button, LV_ALIGN_CENTER, 0, 0);

  // /* Simple UI */
  // lv_obj_t* label = lv_label_create(button);
  // lv_label_set_text(label, "Hello!!!!");

  // /* check button*/
  // lv_obj_add_event(button, [](lv_event_t* e){
  //   Serial.printf("button clicked!!!");
  // }, LV_EVENT_CLICKED, nullptr);

  /* screen */
  static lv_obj_t *screen1 = lv_obj_create(nullptr);
  static lv_obj_t *screen2 = lv_obj_create(nullptr);

  lv_obj_set_style_bg_color(screen1, lv_color_make(255, 0, 0), LV_PART_MAIN);
  /* Button UI*/
  lv_obj_t* button1 = lv_button_create(screen1);
  lv_obj_align(button1, LV_ALIGN_CENTER, 0, 0);

  /* Simple UI */
  lv_obj_t* label1 = lv_label_create(button1);
  lv_label_set_text(label1, "screen 2");

  lv_obj_set_style_bg_color(screen2, lv_color_make(0, 255, 0), LV_PART_MAIN);

  /* Button UI*/
  lv_obj_t* button2 = lv_button_create(screen2);
  lv_obj_align(button2, LV_ALIGN_CENTER, 0, 0);

  /* Simple UI */
  lv_obj_t* label2 = lv_label_create(button2);
  lv_label_set_text(label2, "screen 1");

  lv_screen_load(screen1);

  lv_obj_add_event(
    button1, 
    [](lv_event_t* e){
      lv_screen_load_anim(screen2, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 1000, 100, false);
      // lv_screen_load(screen2);
      Serial.printf("screen 1 clicked!!!");
    }, 
    LV_EVENT_CLICKED, nullptr);
  lv_obj_add_event(
    button2, 
    [](lv_event_t* e){
      lv_screen_load_anim(screen1, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 1000, 100, false);
      // lv_screen_load(screen1);
      Serial.printf("screen 2 clicked!!!");
    }, 
    LV_EVENT_CLICKED, nullptr);
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
  if (!touch.Pressed()) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }
  data->state   = LV_INDEV_STATE_PRESSED;
  data->point.x = touch.X();
  data->point.y = touch.Y();
  
  Serial.printf("x: %d - y: %d \n", data->point.x, data->point.y);
}

static uint32_t my_tick() {
  return millis();
}
