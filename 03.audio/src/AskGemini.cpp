#include "AskGemini.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* GEMINI_API_KEY = "AIzaSyCUN0rPc_TaPx2TUTmSZMY23r0Kypc_Y6Q";
const int GEMINI_MAX_TOKENS = 500;
const char* GEMINI_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=";

// ====== Ask Gemini ======
String askGemini(const String& question) {
    HTTPClient http;
    String url = String(GEMINI_URL) + GEMINI_API_KEY;

    if (!http.begin(url)) {
        return "Không thể kết nối tới máy chủ Gemini.";
    }

    // question = question + "(As brief as possible)";

    http.addHeader("Content-Type", "application/json");
    String payload = "{\"contents\":[{\"parts\":[{\"text\":\"" + question + "\"}]}],\"generationConfig\":{\"maxOutputTokens\":" + String(GEMINI_MAX_TOKENS) + "}}";

    int httpCode = http.POST(payload);
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        return "Lỗi HTTP: " + http.errorToString(httpCode);
    }

    String response = http.getString();
    http.end();

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        return "Lỗi phân tích JSON.";
    }

    String answer = doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
    answer.trim();

    return answer;
}

String cleanTextForTTS(const String& input) {
    String output = "";
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