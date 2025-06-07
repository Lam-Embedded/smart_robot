#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Audio.h>

// ====== WiFi & API Config ======
const char* WIFI_SSID = "Redmi";
const char* WIFI_PASSWORD = "11111111";
const char* GEMINI_API_KEY = "AIzaSyCUN0rPc_TaPx2TUTmSZMY23r0Kypc_Y6Q";
const int GEMINI_MAX_TOKENS = 100;
const char* GEMINI_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=";

// ====== I2S Audio Pins ======
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC  26

Audio audio;

// ====== Function Prototypes ======
String askGemini(const String& question);
String readSerialInput();
void audio_info(const char *info);
String cleanTextForTTS(const String& input);

// ====== Setup ======
void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Audio Setup
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(255);
}

// ====== Main Loop ======
void loop() {
    audio.loop();  // Required to keep audio running

    Serial.println("\nAsk your question:");
    String question = readSerialInput();

    if (question.length() == 0) {
        Serial.println("Empty question. Please type again.");
        return;
    }

    Serial.println("\nSending to Gemini...");
    String response = askGemini(question);

    Serial.println("\nGemini Response:\n" + response);

    // Speak the response
    String cleaned = cleanTextForTTS(response);
    audio.connecttospeech(cleaned.c_str(), "vi");


    // Wait for audio to finish speaking before accepting next input
    while (audio.isRunning()) {
        audio.loop();
        delay(10);
    }
}

// ====== Read Serial Input ======
String readSerialInput() {
    String input = "";

    while (!Serial.available()) {
        audio.loop();  // Keep audio running if idle
        delay(10);
    }

    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') continue;
        input += c;
        delay(1);
    }

    input.trim();
    return input;
}

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

// ====== Audio Info Callback (Optional for Debugging) ======
void audio_info(const char *info) {
    Serial.print("Audio info: ");
    Serial.println(info);
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
    if (output.length() > 200) {
        output = output.substring(0, 200); // Google TTS giới hạn ~200 ký tự
    }
    return output;
}
