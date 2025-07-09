#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Audio.h>
#include <HTTPClient.h>
#include <SD.h>

#include "AskGemini.h"
#include "setUpWifi.h"
#include "textToSpeech.h"
#include "AudioFileSourceHTTPStream.h"
#include "lib_audio_recording.h"
#include "lib_audio_transcription.h"

/*========================= DEFINE SOUCE FILE ===========================*/
#define AUDIO_FILE        "/test_1.wav"

/*========================= DEFINE GPIO_PIN =============================*/
#define pin_RECORD_BTN    4
#define LED               2

/*========================= KHOI TAO MQTT ===============================*/
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "myrobot/esp32/cmd";

/*========================= KHOI TAO WIFI ===============================*/
WiFiClient espClient;
PubSubClient client(espClient);

/*========================= KHOI TAO JSON ===============================*/
StaticJsonDocument<4096> mainDoc;
JsonArray combinedArray = mainDoc.to<JsonArray>();

// const char* ssid = "TP-Link_0368";
// const char* password = "42631958";

Audio audio;

// ============================= FUNC ===============================
bool isButtonPressed();
bool isButtonReleased();
bool shouldPingServer(uint32_t last_ping);
void initPins();
void initSDCard();
void handleRecordingStart();
String handleRecordingStopAndTranscribe();
void indicateReconnect();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
void speech_answer(String input_answer, String segments_answer[], String store_url_answer[], int maxSegments_answer);
int splitText(String input, int wordsPerSegment, String segments[], String store_url[], int maxSegments);

// ============================= SETUP =============================
void setup() {
    Serial.begin(115200);
    wifiSetup();
    Serial.setTimeout(100);

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21);

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
    initPins();
    initSDCard();
    I2S_Record_Init();

    Serial.println("> HOLD button for recording AUDIO .. RELEASE button for REPLAY & Deepgram transcription");
}
bool hasSpoken = false;  // Thêm biến toàn cục

// ============================ LOOP ==============================
void loop() {
    if (!client.connected()) reconnect();
    client.loop();
    audio.loop();

    if (!hasSpoken) {
        hasSpoken = true;

        const int MAX_SEGMENTS = 10;
        String segments[MAX_SEGMENTS];
        String store_url[MAX_SEGMENTS];

        String test = "xin chào, tôi là Lâm";
        speech_answer(test, segments, store_url, MAX_SEGMENTS);
    }

    // const int MAX_SEGMENTS = 10;
    // static uint32_t last_keepalive_time = 0;
    // String segments[MAX_SEGMENTS];
    // String store_url[MAX_SEGMENTS];

    // if (isButtonPressed()) {
    //     handleRecordingStart();
    // }

    // if (isButtonReleased()) {
    //     String text = handleRecordingStopAndTranscribe();
    //     speech_answer(text, segments, store_url, MAX_SEGMENTS);
    // }

    // if (shouldPingServer(last_keepalive_time)) {
    //     last_keepalive_time = millis();
    //     indicateReconnect();
    //     Deepgram_KeepAlive();
    // }
}

void audio_info(const char* info) {
    Serial.print("Audio info: ");
    Serial.println(info);
}

void speech_answer(String input_answer, String segments_answer[], String store_url_answer[], int maxSegments_answer) {
    int numSegments = splitText(input_answer, 20, segments_answer, store_url_answer, maxSegments_answer);

    for (int i = 0; i < numSegments; i++) {
        Serial.println("Speaking segment: " + segments_answer[i]);
        audio.connecttohost(store_url_answer[i].c_str());

        while (audio.isRunning()) {
            audio.loop();
            yield();
        }
    }
}

int splitText(String input, int wordsPerSegment, String segments[], String store_url[], int maxSegments) {
    input.replace("\n", "");
    input.replace("\r", "");

    int wordCount = 0;
    String segment = "";
    int index = 0, segmentIndex = 0;

    while (index < input.length()) {
        int spaceIndex = input.indexOf(' ', index);
        if (spaceIndex == -1) spaceIndex = input.length();

        String word = input.substring(index, spaceIndex);
        segment += word + " ";
        wordCount++;

        if (wordCount == wordsPerSegment && segmentIndex < maxSegments) {
            segments[segmentIndex] = segment;
            store_url[segmentIndex] = getFPTAudioURL(segment);
            if (store_url[segmentIndex] == "") Serial.println("Không lấy được link âm thanh!");
            segment = "";
            wordCount = 0;
            segmentIndex++;
        }

        index = spaceIndex + 1;
    }

    if (segment.length() > 0 && segmentIndex < maxSegments) {
        segments[segmentIndex] = segment;
        store_url[segmentIndex] = getFPTAudioURL(segment);
        if (store_url[segmentIndex] == "") Serial.println("Không lấy được link âm thanh!");
        segmentIndex++;
    }

    return segmentIndex;
}


// ====================== INITIALIZATION ==========================

void initPins() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    pinMode(pin_RECORD_BTN, INPUT_PULLUP);
}

// void connectToWiFi() {
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(ssid, password);

//     Serial.print("Connecting WLAN ");
//     while (WiFi.status() != WL_CONNECTED) {
//         Serial.print(".");
//         delay(500);
//     }
//     Serial.println(". Done, device connected.");
// }

void initSDCard() {
    if (!SD.begin()) {
        Serial.println("ERROR - SD Card initialization failed!");
        while (true);  // Dừng chương trình nếu lỗi
    }
}

// =================== RECORDING & TRANSCRIPTION ===================

void handleRecordingStart() {
    digitalWrite(LED, HIGH);
    delay(15);  // debouncing

    Record_Start(AUDIO_FILE);
}

String handleRecordingStopAndTranscribe() {
    digitalWrite(LED, LOW);

    float recorded_seconds;
    if (Record_Available(AUDIO_FILE, &recorded_seconds)) {
        if (recorded_seconds > 0.2) {
            digitalWrite(LED, HIGH);
            String transcription = SpeechToText_Deepgram(AUDIO_FILE);
            digitalWrite(LED, LOW);

            if (transcription.length() == 0) {
                Serial.println("Empty question. Please type again.");
                transcription = "hello";
                return transcription;
            }
            
            transcription += " (câu trả lời theo văn phong cách nói chuyên tự nhiên, dễ thương, ngắn ngọn xúc tích nhất có thể, số từ nhỏ hơn 200, viết liền mạch)";

            Serial.println("\nSending to Gemini...");
            String response = askGemini(transcription) + " Hết";
            Serial.println("\nGemini Response:\n" + response);

            return response;  
        }
    }
}

// ======================== HELPERS ==============================

bool isButtonPressed() {
    return digitalRead(pin_RECORD_BTN) == LOW;
}

bool isButtonReleased() {
    return digitalRead(pin_RECORD_BTN) == HIGH;
}

bool shouldPingServer(uint32_t last_ping) {
    return isButtonReleased() && !audio.isRunning() && (millis() - last_ping > 5000);
}

void indicateReconnect() {
    digitalWrite(LED, HIGH);
}

void callback(char* topic, byte* message, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) msg += (char)message[i];

    Serial.println("📩 Nhận được JSON mới:");
    Serial.println(msg);

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if (error) {
        Serial.print("❌ Lỗi JSON: ");
        Serial.println(error.c_str());
        return;
    }

    if (doc.is<JsonArray>()) {
        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject item : arr) combinedArray.add(item);
        Serial.println("✅ Đã thêm các phần tử từ mảng JSON.");
    } else if (doc.is<JsonObject>()) {
        combinedArray.add(doc.as<JsonObject>());
        Serial.println("✅ Đã thêm một đối tượng JSON.");
    } else {
        Serial.println("❌ Không phải mảng hay đối tượng JSON.");
    }

    Serial.print("📌 Tổng số công việc đã nhận: ");
    Serial.println(combinedArray.size());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("🔁 Đang kết nối MQTT...");
        String clientId = "ESP32Client-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("✅ Thành công");
            client.subscribe(mqtt_topic);
        } else {
            Serial.print("❌ Thất bại. Lỗi: ");
            Serial.print(client.state());
            Serial.println(" -> thử lại sau 5 giây");
            delay(5000);
        }
    }
}
