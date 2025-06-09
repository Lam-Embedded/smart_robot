#include "textToSpeech.h"
#include "AudioFileSourceHTTPStream.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include <ArduinoJson.h>

// FPT API info
const char* api_key = "bHj3hY6gdoKl62Kvnq0vklsVICKFtUtR";
const char* tts_api_url = "https://api.fpt.ai/hmi/tts/v5";

String getFPTAudioURL(String text) {
    HTTPClient http;
    http.begin(tts_api_url);
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
            Serial.println("JSON parse error");
            return "";
        }
        
        return doc["async"].as<String>();
    } else {
        Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpCode).c_str());
        return "";
    }
}
