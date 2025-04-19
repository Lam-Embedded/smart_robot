#include <Arduino.h>
#include <WiFi.h>
#include "SPIFFS.h"

// Cấu hình WiFi
const char* ssid = "HelloWorld";
const char* password = "1355311351";

// Cấu hình các chân GPIO
const int output2 = 2;
const int output27 = 27;

String output2State = "off";
String output27State = "off";

WiFiServer server(80);
void setupWifi();

void setup() {
    Serial.begin(9600);
    server.begin();

    pinMode(output2, OUTPUT);
    pinMode(output27, OUTPUT);
    digitalWrite(output2, LOW);
    digitalWrite(output27, LOW);

    // Khởi động SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Lỗi khi khởi động SPIFFS");
        return;
    }

    // Kết nối WiFi
    setupWifi();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("Client mới kết nối.");
        String currentLine = "";
        String header = "";

        unsigned long timeout = millis();
        while (client.connected() && millis() - timeout < 2000) {
            if (client.available()) {
                char c = client.read();
                header += c;
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        // Xử lý yêu cầu
                        if (header.indexOf("GET /2/on") >= 0) {
                            digitalWrite(output2, HIGH);
                            output2State = "on";
                        } else if (header.indexOf("GET /2/off") >= 0) {
                            digitalWrite(output2, LOW);
                            output2State = "off";
                        } else if (header.indexOf("GET /27/on") >= 0) {
                            digitalWrite(output27, HIGH);
                            output27State = "on";
                        } else if (header.indexOf("GET /27/off") >= 0) {
                            digitalWrite(output27, LOW);
                            output27State = "off";
                        }
                      
                        if (header.indexOf("GET /style.css") >= 0) {
                            File cssFile = SPIFFS.open("/style.css", "r");
                            if (cssFile) {
                                client.println("HTTP/1.1 200 OK");
                                client.println("Content-type: text/css");
                                client.println("Connection: close");
                                client.println();
                                
                                while (cssFile.available()) {
                                    client.write(cssFile.read());
                                }
                                cssFile.close();
                            }
                        } else {
                            File htmlFile = SPIFFS.open("/index.html", "r");
                            if (htmlFile) {
                                client.println("HTTP/1.1 200 OK");
                                client.println("Content-type: text/html");
                                client.println("Connection: close");
                                client.println();
                                
                                while (htmlFile.available()) {
                                    String line = htmlFile.readStringUntil('\n');
                                    line.replace("%STATE2%", output2State);
                                    line.replace("%STATE27%", output27State);
                                    client.print(line);
                                }
                                htmlFile.close();
                            }
                        }
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                  currentLine += c;
                }
            }
        }
        delay(1);
        client.stop();
        Serial.println("Client disconected.");
    }
}

void setupWifi() {
    WiFi.begin(ssid, password);
    Serial.print("Conecting to wifi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}
