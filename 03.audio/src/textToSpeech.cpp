#include "textToSpeech.h"
#include "AudioFileSourceHTTPStream.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// FPT API info
const char* api_key = "bHj3hY6gdoKl62Kvnq0vklsVICKFtUtR";
const char* tts_api_url = "https://api.fpt.ai/hmi/tts/v5";

String getFPTAudioURL(String text) {
    WiFiClientSecure client;
    client.setInsecure();  // ⚠️ Không kiểm tra chứng chỉ SSL, dùng khi không có CA cert
    HTTPClient http;
    http.setTimeout(5000); // timeout 5s
    http.begin(client, tts_api_url);

    http.addHeader("api_key", api_key);
    http.addHeader("voice", "banmai");
    http.addHeader("speed", "0");
    http.addHeader("format", "mp3");

    int httpCode = http.POST(text.c_str());

    if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("FPT Response: " + payload);

        DynamicJsonDocument doc(1024);
        DeserializationError err = deserializeJson(doc, payload);
        if (err) {
            Serial.println("❌ JSON parse error");
            return "";
        }

        if (!doc.containsKey("async")) {
            Serial.println("❌ Không tìm thấy trường async trong JSON!");
            return "";
        }

        return doc["async"].as<String>();
    } else {
        Serial.printf("❌ HTTP POST failed, error: %s\n", http.errorToString(httpCode).c_str());
        return "";
    }
}
