/*
    - Thiet lap ESP o che do bat wifi ben ngoai
*/
#include <Arduino.h>
#include <WiFi.h>
#include <stdio.h>

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_STA);    // Thiet lap esp o che do STA
    WiFi.disconnect();      // Xoa cac ket noi truoc do

    Serial.println("Setup done!!!!");
    delay(2000);
}

void loop() {
    Serial.println("Start scan!!!");
    // Ham WiFi.scanNetworks(); tra ve so wifi hien co
    int wifiScan = WiFi.scanNetworks();

    if (wifiScan == 0){
        Serial.println("No wifi!!!");
    }
    else {
        // WiFi.SSID(i): Tra ve ten wifi
        // WiFi.RSSI(i): Tra ve chat luong wifi
        Serial.printf("Found: %d wifi\n", wifiScan);
        for (int i = 0; i < wifiScan; i++){
            Serial.printf("%d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }
    }
    delay(5000);
}
