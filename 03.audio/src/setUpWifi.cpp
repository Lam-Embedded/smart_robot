#include <WiFiManager.h>
#include "setUpWifi.h"

IPAddress local_IP(192, 168, 0, 142);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

void wifiSetup() {
    WiFi.mode(WIFI_STA);
    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);

    WiFiManager wm;
    if (!wm.autoConnect("AutoConnectAP_heart_rate")) {
        Serial.println("Failed to connect");
        ESP.restart();
    } else {
        Serial.println("WiFi connected!");
    }

    Serial.println(WiFi.localIP());
}