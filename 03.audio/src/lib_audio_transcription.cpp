#include <WiFiClientSecure.h>
#include <SD.h>
#include "lib_audio_transcription.h"

// --- Debug Macro ---
#ifndef DEBUG
#define DEBUG true
#endif

#if DEBUG
#define DebugPrint(x) Serial.print(x)
#define DebugPrintln(x) Serial.println(x)
#else
#define DebugPrint(x)
#define DebugPrintln(x)
#endif

// --- Configurations ---
#ifndef DEEPGRAM_API_KEY
#error "DEEPGRAM_API_KEY is not defined! Please define in platformio.ini"
#endif

const char* deepgramApiKey = DEEPGRAM_API_KEY;

#define STT_LANGUAGE        "vi"
#define TIMEOUT_DEEPGRAM    12
#define STT_KEYWORDS        "&keywords=KALO&keywords=Janthip&keywords=Google"

WiFiClientSecure client_deepgram;

String SpeechToText_Deepgram(const String& audio_filename) {
    uint32_t t_start = millis();

    // --- Ensure Connection ---
    if (!client_deepgram.connected()) {
        DebugPrintln("> Connecting to Deepgram server...");
        client_deepgram.setInsecure();
        if (!client_deepgram.connect("api.deepgram.com", 443)) {
            Serial.println("ERROR - Failed to connect to Deepgram");
            client_deepgram.stop();
            return "";
        }
        DebugPrintln("Connected.");
    }

    // --- Open Audio File ---
    File audioFile = SD.open(audio_filename);
    if (!audioFile) {
        Serial.println("ERROR - Failed to open audio file");
        return "";
    }
    size_t audio_size = audioFile.size();
    audioFile.close();
    DebugPrintln("> Audio file: " + audio_filename + " | Size: " + String(audio_size));

    // --- Flush Previous Data ---
    while (client_deepgram.available()) client_deepgram.read();

    // --- Build Request ---
    String url = "?model=nova-2-general";
    url += (strlen(STT_LANGUAGE) > 0) ? "&language=" + String(STT_LANGUAGE) : "&detect_language=true";
    url += "&smart_format=true&numerals=true";
    url += STT_KEYWORDS;

    client_deepgram.println("POST /v1/listen" + url + " HTTP/1.1");
    client_deepgram.println("Host: api.deepgram.com");
    client_deepgram.println("Authorization: Token " + String(deepgramApiKey));
    client_deepgram.println("Content-Type: audio/wav");
    client_deepgram.println("Content-Length: " + String(audio_size));
    client_deepgram.println();

    // --- Send Audio in Chunks ---
    File file = SD.open(audio_filename, FILE_READ);
    const size_t bufferSize = 1024;
    uint8_t buffer[bufferSize];
    size_t bytesRead;
    while ((bytesRead = file.read(buffer, bufferSize)) > 0) {
        client_deepgram.write(buffer, bytesRead);
    }
    file.close();
    DebugPrintln("> Audio data sent.");

    // --- Wait for Response ---
    String response;
    uint32_t timeout = millis() + TIMEOUT_DEEPGRAM * 1000;
    while (response.isEmpty() && millis() < timeout) {
        while (client_deepgram.available()) {
            response += static_cast<char>(client_deepgram.read());
        }
        delay(100);
        DebugPrint(".");
    }
    DebugPrintln();

    if (millis() >= timeout) {
        Serial.println("*** TIMEOUT waiting Deepgram response ***");
    }

    client_deepgram.stop();  // Close to allow audio playback elsewhere

    // --- Parse JSON Response ---
    String transcription = json_object(response, "\"transcript\":");
    String language = json_object(response, "\"detected_language\":");
    String duration = json_object(response, "\"duration\":");

    DebugPrintln("=== Deepgram Response Summary ===");
    DebugPrintln("Detected Language: " + language);
    DebugPrintln("Duration: " + duration + " sec");
    DebugPrintln("Transcription: " + transcription);
    DebugPrintln("=================================");

    return transcription;
}

void Deepgram_KeepAlive() {
    uint32_t t_start = millis();
    DebugPrint("* Deepgram KeepAlive | ");

    if (!client_deepgram.connected()) {
        DebugPrint("Reconnecting... ");
        client_deepgram.setInsecure();
        if (!client_deepgram.connect("api.deepgram.com", 443)) {
            Serial.println("ERROR - Reconnect failed");
            return;
        }
        DebugPrintln("Connected.");
        return;
    }

    // --- Send Dummy Silent WAV ---
    uint8_t silent_wav[] = {
        0x52,0x49,0x46,0x46,0x40,0x00,0x00,0x00,0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
        0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x80,0x3E,0x00,0x00,0x80,0x3E,0x00,0x00,
        0x01,0x00,0x08,0x00,0x64,0x61,0x74,0x61,0x14,0x00,0x00,0x00,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    };

    client_deepgram.println("POST /v1/listen HTTP/1.1");
    client_deepgram.println("Host: api.deepgram.com");
    client_deepgram.println("Authorization: Token " + String(deepgramApiKey));
    client_deepgram.println("Content-Type: audio/wav");
    client_deepgram.println("Content-Length: " + String(sizeof(silent_wav)));
    client_deepgram.println();
    client_deepgram.write(silent_wav, sizeof(silent_wav));

    // --- Read Response ---
    String response;
    while (client_deepgram.available()) response += static_cast<char>(client_deepgram.read());

    DebugPrintln("KeepAlive Sent | RX bytes: " + String(response.length()) + 
                 " | Time: " + String((float)(millis() - t_start) / 1000) + "s");
}

String json_object(const String& input, const String& element) {
    int pos_start = input.indexOf(element);
    if (pos_start < 0) return "";

    pos_start += element.length();
    int pos_end = input.indexOf(",\"", pos_start);
    if (pos_end < 0) return "";

    String content = input.substring(pos_start, pos_end);
    content.trim();

    if (content.startsWith("\"") && content.endsWith("\"")) {
        content = content.substring(1, content.length() - 1);
    }
    return content;
}
