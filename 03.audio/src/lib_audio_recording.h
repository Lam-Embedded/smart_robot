#ifndef LIB_AUDIO_RECORDING_H
#define LIB_AUDIO_RECORDING_H

#include <Arduino.h>
#include <SD.h>  // Dùng để ghi âm ra thẻ SD

/**
 * @brief Khởi tạo giao tiếp I2S với microphone INMP441.
 * 
 * @return true nếu khởi tạo thành công, false nếu lỗi.
 */
bool I2S_Record_Init();

/**
 * @brief Bắt đầu ghi âm (tạo file WAV mới) hoặc tiếp tục ghi âm vào file.
 * 
 * @param audio_filename Tên file WAV để ghi âm, ví dụ "/recording.wav"
 * @return true nếu thành công, false nếu lỗi khi tạo hoặc ghi file.
 */
bool Record_Start(const String& audio_filename);

/**
 * @brief Kết thúc ghi âm, cập nhật header WAV và tính toán thời lượng ghi.
 * 
 * @param audio_filename Tên file WAV vừa ghi.
 * @param audiolength_sec Con trỏ lưu thời lượng ghi âm (tính bằng giây).
 * @return true nếu cập nhật header thành công, false nếu lỗi mở file.
 */
bool Record_Available(const String& audio_filename, float* audiolength_sec);

#endif  // LIB_AUDIO_RECORDING_H
