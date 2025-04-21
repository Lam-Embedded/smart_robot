#include <Arduino.h>
#include <WiFi.h>
#include <stdio.h>

const char *ssid = "FOUR SEASONS";
const char *password = "";

IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);


void setupWifi();
void scanWifi();

void setup() {
    Serial.begin(9600);
    WiFi.mode(WIFI_STA);
    // setupWifi();
    delay(2000);
}

void loop() {
    
}

void setupWifi() {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA fail to configure!!!");
    }
    WiFi.mode(WIFI_STA);    // Thiet lap esp o che do STA
    WiFi.begin(ssid, password);
    Serial.print("Conecting to wifi ..");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("Setup done!!!!");
    Serial.println(WiFi.localIP());
    Serial.printf("RSSI: %d", WiFi.RSSI());
}

void scanWifi() {
    Serial.println("Start scan!!!");
    // Ham WiFi.scanNetworks(); tra ve so wifi hien co
    int wifiScan = WiFi.scanNetworks();

    if (wifiScan == 0) {
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