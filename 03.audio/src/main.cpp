#include <Arduino.h>
#include <WiFi.h>
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

// ====== Function Prototypes ======
void audio_info(const char *info);
String readSerialInput();
int splitText(String input, int wordsPerSegment, String segments[], String store_url[], int maxSegments);

// ====== Setup ======
void setup() {
    Serial.begin(115200);
    wifiSetup();

    // Audio Setup
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(255);
}

// ====== Main Loop ======
void loop() {
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
