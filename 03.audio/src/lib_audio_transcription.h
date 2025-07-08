#pragma once

#include <Arduino.h>
#include <SD.h>

/**
 * @brief Gửi file WAV lên Deepgram để chuyển giọng nói thành văn bản.
 * 
 * @param audio_filename Tên file WAV trên thẻ SD (ví dụ: "/record.wav").
 * @return String Văn bản đã được chuyển đổi, hoặc chuỗi rỗng nếu lỗi.
 */
String SpeechToText_Deepgram(const String& audio_filename);

/**
 * @brief Gửi gói keep-alive (âm thanh im lặng) đến Deepgram để giữ kết nối mở.
 */
void Deepgram_KeepAlive();

/**
 * @brief Trích xuất giá trị từ chuỗi JSON theo một khóa nhất định.
 * 
 * @param input Dữ liệu JSON dạng chuỗi.
 * @param element Tên khóa cần lấy giá trị (ví dụ: "\"transcript\":").
 * @return String Giá trị tương ứng với khóa hoặc chuỗi rỗng nếu không tìm thấy.
 */
String json_object(const String& input, const String& element);
