#include "wifi_manager.h"
#include <Arduino.h>

void initWiFi() {
    // Загрузка учетных данных из .env
    const char* ssid = WIFI_SSID;
    const char* password = WIFI_PASSWORD;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void ensureWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost. Reconnecting...");
        WiFi.disconnect();
        initWiFi();
    }
} 