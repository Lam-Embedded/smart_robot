/**
 * @file AskGemini.h
 * @brief Khai báo các hàm giao tiếp với API Gemini và xử lý văn bản TTS.
 */

#ifndef ASK_GEMINI_H
#define ASK_GEMINI_H

#include <Arduino.h>

/**
 * @brief Gửi một câu hỏi đến API Gemini và trả về phản hồi.
 * 
 * @param question Chuỗi văn bản câu hỏi.
 * @return Chuỗi phản hồi từ Gemini.
 */
String askGemini(const String& question);

/**
 * @brief Làm sạch văn bản để phù hợp hơn với Text-to-Speech.
 * 
 * @param input Văn bản đầu vào từ Gemini.
 * @return Văn bản đã được lọc ký tự không phù hợp.
 */
String cleanTextForTTS(const String& input);

#endif  // ASK_GEMINI_H
