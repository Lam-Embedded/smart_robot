/**
 * @file lib_audio_recording.cpp
 * @brief Ghi âm từ microphone I2S (INMP441) và lưu vào thẻ nhớ dưới định dạng WAV.
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include "lib_audio_recording.h"
#include <SD.h>

// ======================== Debug Macro ========================
#ifndef DEBUG
  #define DEBUG true
#endif

#define DebugPrint(x)     if (DEBUG) { Serial.print(x); }
#define DebugPrintln(x)   if (DEBUG) { Serial.println(x); }

// ====================== I2S Pin Assignments ===================
#define I2S_WS     22   // Word Select (LRCK)
#define I2S_SD     35   // Serial Data In
#define I2S_SCK    33   // Serial Clock

// ====================== Audio Config ==========================
#define SAMPLE_RATE        16000
#define BITS_PER_SAMPLE    16
#define GAIN_BOOSTER_I2S   45   // Khuếch đại tín hiệu (1–64)

const i2s_port_t I2S_PORT = I2S_NUM_1;

// ====================== WAV Header Format =====================
struct WAV_HEADER {
  char  riff[4]         = { 'R', 'I', 'F', 'F' };
  long  flength         = 0;
  char  wave[4]         = { 'W', 'A', 'V', 'E' };
  char  fmt[4]          = { 'f', 'm', 't', ' ' };
  long  chunk_size      = 16;
  short format_tag      = 1;    // PCM
  short num_channels    = 1;    // Mono
  long  sample_rate     = SAMPLE_RATE;
  long  bytes_per_sec   = SAMPLE_RATE * (BITS_PER_SAMPLE / 8);
  short bytes_per_samp  = (BITS_PER_SAMPLE / 8);
  short bits_per_samp   = BITS_PER_SAMPLE;
  char  data[4]         = { 'd', 'a', 't', 'a' };
  long  data_length     = 0;
} myWAV_Header;

// ====================== Trạng thái ghi âm =====================
bool is_recording = false;
bool i2s_initialized = false;
static File recording_file;  // File ghi âm duy trì mở liên tục

// =============================================================
// =============== Hàm Khởi tạo I2S để thu âm ===================
// =============================================================
bool I2S_Record_Init() {
  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S
,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  if (i2s_driver_install(I2S_PORT, &config, 0, NULL) != ESP_OK) {
    Serial.println("[I2S] Không thể cài đặt driver I2S");
    return false;
  }

  if (i2s_set_pin(I2S_PORT, &pin_config) != ESP_OK) {
    Serial.println("[I2S] Không thể thiết lập chân I2S");
    return false;
  }

  DebugPrintln("[I2S] I2S khởi tạo thành công");
  i2s_initialized = true;
  return true;
}

// =============================================================
// ========== Bắt đầu hoặc tiếp tục ghi dữ liệu I2S ============
// =============================================================
bool Record_Start(const String& filename) {
  if (!i2s_initialized) {
    Serial.println("[Ghi âm] I2S chưa được khởi tạo.");
    return false;
  }

  if (!is_recording) {
    // --- Bắt đầu ghi mới: tạo file và ghi header WAV ---
    is_recording = true;

    if (SD.exists(filename)) SD.remove(filename);

    recording_file = SD.open(filename, FILE_WRITE);
    if (!recording_file) {
      Serial.println("[Ghi âm] Không thể tạo file.");
      return false;
    }

    recording_file.write((uint8_t*)&myWAV_Header, sizeof(WAV_HEADER));
    recording_file.flush();  // Đảm bảo header được ghi
    DebugPrintln("> Bắt đầu ghi âm, đã ghi header WAV.");
    return true;
  }

  // --- Đọc và ghi dữ liệu mẫu âm thanh ---
  int16_t buffer[1024];  // Mỗi mẫu 2 byte
  size_t bytes_read = 0;

  i2s_read(I2S_PORT, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);

  // Khuếch đại tín hiệu nếu cần
  if (GAIN_BOOSTER_I2S > 1 && GAIN_BOOSTER_I2S <= 64) {
    for (size_t i = 0; i < bytes_read / 2; i++) {
      buffer[i] = buffer[i] * GAIN_BOOSTER_I2S;
    }
  }

  if (!recording_file) {
    Serial.println("[Ghi âm] File không sẵn sàng để ghi.");
    return false;
  }

  recording_file.write((uint8_t*)buffer, bytes_read);
  recording_file.flush();  // Đảm bảo ghi ngay
  return true;
}

// =============================================================
// ====== Dừng ghi âm, cập nhật header và tính thời lượng =======
// =============================================================
bool Record_Available(const String& filename, float* duration_sec) {
  if (!is_recording || !i2s_initialized) return false;

  if (!recording_file) {
    Serial.println("[Finalize] File ghi âm không mở.");
    return false;
  }

  long filesize = recording_file.size();

  // Cập nhật thông tin vào header WAV
  myWAV_Header.flength = filesize;
  myWAV_Header.data_length = filesize - sizeof(WAV_HEADER);

  recording_file.seek(0);
  recording_file.write((uint8_t*)&myWAV_Header, sizeof(WAV_HEADER));
  recording_file.close();  // Đóng file
  is_recording = false;

  *duration_sec = (float)(filesize - sizeof(WAV_HEADER)) / (SAMPLE_RATE * (BITS_PER_SAMPLE / 8));

  DebugPrintln("> Ghi âm hoàn tất.");
  DebugPrint("> File: '" + filename + "', Kích thước: " + String(filesize) + " bytes, Thời lượng: " + String(*duration_sec) + " giây");

  return true;
}
