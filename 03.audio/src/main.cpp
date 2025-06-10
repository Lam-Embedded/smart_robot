#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Audio.h>
#include <HTTPClient.h>
#include "AskGemini.h"
#include "setUpWifi.h"
#include "textToSpeech.h"
#include "AudioFileSourceHTTPStream.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

Audio audio;

AudioGeneratorMP3 *mp3;
AudioFileSourceHTTPStream *file;
AudioOutputI2S *out;

//==============MQTT================

// ====== Function Prototypes ======
void audio_info(const char *info);
String readSerialInput();
int splitText(String input, int wordsPerSegment, String segments[], String store_url[], int maxSegments);
void reconnect();
void callback(char* topic, byte* message, unsigned int length);

// ====== Setup ======
void setup() {
    Serial.begin(115200);
    wifiSetup();

    // Audio Setup
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(255);

    // Thiết lập MQTT
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

// ====== Main Loop ======
void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    static const int MAX_SEGMENTS = 10;
    String segments[MAX_SEGMENTS];
    String store_url[MAX_SEGMENTS];

    audio.loop();  // Keep audio running

    Serial.println("\nAsk your question:");
    String question = readSerialInput();
    question += " (câu trả lời ngắn ngọn xúc tích nhất có thể, số từ nhỏ hơn 200, viết liền mạch)";

    if (question.length() == 0) {
        Serial.println("Empty question. Please type again.");
        return;
    }

    Serial.println("\nSending to Gemini...");
    String response = askGemini(question) + " Hết";

    Serial.println("\nGemini Response:\n" + response);

    int numSegments = splitText(response, 20, segments, store_url, MAX_SEGMENTS);

    for (int i = 0; i < numSegments; i++) {
        Serial.println("Speaking segment: " + segments[i]);
        audio.connecttohost(store_url[i].c_str());

        while (audio.isRunning()) {
            audio.loop();
            delay(10);
        }
    }
}


// ====== Audio Info Callback (Optional for Debugging) ======
void audio_info(const char *info) {
    Serial.print("Audio info: ");
    Serial.println(info);
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

int splitText(String input, int wordsPerSegment, String segments[], String store_url[], int maxSegments) {
    input.replace("\n", "");
    input.replace("\r", "");

    int wordCount = 0;
    String segment = "";
    int index = 0;
    int segmentIndex = 0;

    while (index < input.length()) {
        int spaceIndex = input.indexOf(' ', index);
        if (spaceIndex == -1) spaceIndex = input.length();

        String word = input.substring(index, spaceIndex);
        segment += word + " ";
        wordCount++;

        if (wordCount == wordsPerSegment) {
            if (segmentIndex < maxSegments) {
                segments[segmentIndex] = segment;
                store_url[segmentIndex] = getFPTAudioURL(segment);
                if (store_url[segmentIndex] == "") {
                    Serial.println("Không lấy được link âm thanh!");
                }
                segmentIndex++;
            }
            segment = "";
            wordCount = 0;
        }

        index = spaceIndex + 1;
    }

    // Lưu đoạn cuối nếu còn
    if (segment.length() > 0 && segmentIndex < maxSegments) {
        segments[segmentIndex] = segment;
        store_url[segmentIndex] = getFPTAudioURL(segment);
        if (store_url[segmentIndex] == "") {
            Serial.println("Không lấy được link âm thanh!");
        }
        segmentIndex++;
    }

    return segmentIndex;
}

void reconnect() {
  // Lặp lại cho đến khi kết nối lại được
  while (!client.connected()) {
    Serial.print("🔁 Đang kết nối MQTT...");
    // Tên client ngẫu nhiên để tránh trùng
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
  Serial.print("📩 Tin nhắn nhận được [");
  Serial.print(topic);
  Serial.print("]: ");

  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.println(msg);
  
  // Ở đây bạn có thể xử lý lệnh, ví dụ nếu msg == "on", bật LED chẳng hạn
}
