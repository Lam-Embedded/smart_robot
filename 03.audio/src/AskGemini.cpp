/**
 * @file AskGemini.cpp
 * @brief Gửi câu hỏi đến API Gemini và nhận phản hồi văn bản, đồng thời hỗ trợ làm sạch văn bản cho TTS.
 */

#include "AskGemini.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Constants (nên khai báo trong platformio.ini)
#ifndef GEMINI_API_KEY
  #error "GEMINI_API_KEY chưa được khai báo trong platformio.ini"
#endif

const char* GEMINI_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=";
const int GEMINI_MAX_TOKENS = 500;

/**
 * @brief Gửi câu hỏi tới Gemini API và nhận câu trả lời.
 * 
 * @param question Câu hỏi dạng chuỗi.
 * @return Câu trả lời từ Gemini hoặc thông báo lỗi.
 */
String askGemini(const String& question) {
    HTTPClient http;
    String url = String(GEMINI_URL) + GEMINI_API_KEY;

    if (!http.begin(url)) {
        return F("Không thể kết nối tới máy chủ Gemini.");
    }

    http.addHeader("Content-Type", "application/json");

    // JSON payload gửi lên Gemini
    String payload;
    payload.reserve(512); // Tối ưu cấp phát bộ nhớ
    payload = "{\"contents\":[{\"parts\":[{\"text\":\"" + question + "\"}]}],";
    payload += "\"generationConfig\":{\"maxOutputTokens\":" + String(GEMINI_MAX_TOKENS) + "}}";

    int httpCode = http.POST(payload);
    if (httpCode != HTTP_CODE_OK) {
        String errorMsg = "Lỗi HTTP: " + http.errorToString(httpCode);
        http.end();
        return errorMsg;
    }

    String response = http.getString();
    http.end();

    DynamicJsonDocument doc(2048); // Có thể điều chỉnh kích thước nếu response lớn
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        return F("Lỗi phân tích JSON.");
    }

    // Trích xuất câu trả lời
    JsonVariant text = doc["candidates"][0]["content"]["parts"][0]["text"];
    if (text.isNull()) {
        return F("Không có phản hồi từ Gemini.");
    }

    String answer = text.as<String>();
    answer.trim();
    return answer;
}

/**
 * @brief Làm sạch văn bản từ Gemini để phù hợp với TTS (loại bỏ ký tự đặc biệt).
 * 
 * @param input Văn bản đầu vào.
 * @return Văn bản đã làm sạch.
 */
String cleanTextForTTS(const String& input) {
    String output;
    output.reserve(input.length());

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        if (isPrintable(c) && c != '*' && c != '`') {
            output += c;
        } else {
            output += ' ';
        }
    }

    output.trim();
    return output;
}
