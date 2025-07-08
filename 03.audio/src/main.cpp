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
#include "driver/i2s.h"
#include "lib_audio_recording.h"
#include "lib_audio_transcription.h"

#define AUDIO_FILE "/Audio.wav"
#define PIN_RECORD_BTN 14
#define LED_PIN 2
#define AUDIO_FILE        "/test_1.wav"
#define pin_RECORD_BTN    4
#define LED               2

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "myrobot/esp32/cmd";

WiFiClient espClient;
PubSubClient client(espClient);

Audio audio;
Audio audio_play;

StaticJsonDocument<4096> mainDoc;
JsonArray combinedArray = mainDoc.to<JsonArray>();

void audio_info(const char* info);
String readSerialInput();
int splitText(String input, int wordsPerSegment, String segments[], String store_url[], int maxSegments);
void reconnect();
void callback(char* topic, byte* message, unsigned int length);
void speech_answer(String input_answer, String segments_answer[], String store_url_answer[], int maxSegments_answer);
void speech_schedule(String input_schedule, String segments_schedule[], String store_url_schedule[], int maxSegments_schedule);
String update_data();
String Speech_to_text();
void initPins();
void initSDCard();
bool isButtonPressed();
void handleRecordingStart();
bool isButtonReleased();
String handleRecordingStopAndTranscribe();
bool shouldPingServer(uint32_t last_ping);
void indicateReconnect();

void setup() {
    Serial.begin(115200);
    wifiSetup();
    Serial.setTimeout(100);

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(255);

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    initPins();
    initSDCard();
    I2S_Record_Init();

    Serial.println("> HOLD button for recording AUDIO .. RELEASE button for REPLAY & Deepgram transcription");
}

void loop() {
    if (!client.connected()) reconnect();
    client.loop();

    const int MAX_SEGMENTS = 10;
    static uint32_t last_keepalive_time = 0;
    String segments[MAX_SEGMENTS];
    String store_url[MAX_SEGMENTS];

    String segments_schedule[MAX_SEGMENTS];
    String store_url_schedule[MAX_SEGMENTS];
    String data_schedule = update_data();

    audio.loop();

    Serial.println("\nAsk your question:");
    String question = handleRecordingStopAndTranscribe();
    question += " (câu trả lời ngắn ngọn xúc tích nhất có thể, số từ nhỏ hơn 200, viết liền mạch)";

    if (question.length() == 0) {
        Serial.println("Empty question. Please type again.");
        return;
    }

    Serial.println("\nSending to Gemini...");
    String response = askGemini(question) + " Hết";
    Serial.println("\nGemini Response:\n" + response);

    //speech_answer(response, segments, store_url, MAX_SEGMENTS);
    //speech_schedule(data_schedule, segments_schedule, store_url_schedule, MAX_SEGMENTS);
}

void speech_answer(String input_answer, String segments_answer[], String store_url_answer[], int maxSegments_answer) {
    int numSegments = splitText(input_answer, 20, segments_answer, store_url_answer, maxSegments_answer);

    for (int i = 0; i < numSegments; i++) {
        Serial.println("Speaking segment: " + segments_answer[i]);
        audio.connecttohost(store_url_answer[i].c_str());

        while (audio.isRunning()) {
            audio.loop();
            delay(10);
        }
    }
}

void speech_schedule(String input_schedule, String segments_schedule[], String store_url_schedule[], int maxSegments_schedule) {
    int numSegments = splitText(input_schedule, 20, segments_schedule, store_url_schedule, maxSegments_schedule);

    for (int i = 0; i < numSegments; i++) {
        Serial.println("Speaking segment: " + segments_schedule[i]);
        audio.connecttohost(store_url_schedule[i].c_str());

        while (audio.isRunning()) {
            audio.loop();
            delay(10);
        }
    }
}

void audio_info(const char* info) {
    Serial.print("Audio info: ");
    Serial.println(info);
}

String readSerialInput() {
    String input = "";
    while (!Serial.available()) {
        audio.loop();
        delay(10);
    }

    while (Serial.available()) {
        char c = Serial.read();
        if (c != '\n' && c != '\r') input += c;
        delay(1);
    }

    input.trim();
    return input;
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

String update_data() {
    String data = "";
    for (JsonObject item : combinedArray) {
        data += "📝 Tên: " + String(item["Công việc"] | "") +
                " | Thời gian: " + String(item["Thời gian"] | "") +
                " | Ngày: " + String(item["Ngày"] | "") +
                " | Nội dung: " + String(item["ghi chu"] | "") + "; ";
    }

    data = "hãy tạo đoạn văn để nói (dạng robot báo cáo) về các lịch trình như trong bảng sau: " + data;
    return askGemini(data);
}

String Speech_to_text() {
    while (digitalRead(PIN_RECORD_BTN) != LOW);
    digitalWrite(LED_PIN, HIGH);

    Record_Start(AUDIO_FILE);
    Serial.println("> Recording...");

    while (digitalRead(PIN_RECORD_BTN) == LOW) delay(10);
    digitalWrite(LED_PIN, LOW);

    float duration;
    if (Record_Available(AUDIO_FILE, &duration) && duration > 0.4) {
        Serial.println("> Audio saved. Transcribing...");
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("❌ WiFi disconnected.");
            return "";
        }

        String transcription = SpeechToText_Deepgram(AUDIO_FILE);
        Serial.println("> Transcription:");
        Serial.println(transcription);
        return transcription;
    }

    Serial.println("> Recording too short.");
    return "";
}


// ====================== INITIALIZATION ==========================

void initPins() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    pinMode(pin_RECORD_BTN, INPUT_PULLUP);
}

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

            Serial.println(transcription);
            return transcription;
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
    return isButtonReleased() && !audio_play.isRunning() && (millis() - last_ping > 5000);
}

void indicateReconnect() {
    digitalWrite(LED, HIGH);
}
