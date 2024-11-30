// main.cpp
#include <Arduino.h>
#include <M5Unified.h>
#include "config.h"
#include <WiFi.h>
#include "wifisetup.h"

// Interval for checking WiFi status (in milliseconds)
const unsigned long WIFI_CHECK_INTERVAL = 5000; // 5 seconds

// Variables to track WiFi status
unsigned long previousWiFiCheck = 0;
bool lastWiFiStatus = false; // false: not connected, true: connected

void setup(void)
{
    // Initialize Serial for debugging
    Serial.begin(9600);
    while (!Serial); // Wait for Serial to initialize (optional)

    // Initialize M5Unified with default configuration
    auto cfg = M5.config();
    M5.begin(cfg);

    // Load configuration
    if (!loadConfig()) {
        Serial.println("Failed to load configuration. Using default settings.");
    }

    // Initialize WiFi
    bool connected = initWiFi();
    lastWiFiStatus = connected;
}

void loop() {
    M5.update(); // Handle background tasks

    unsigned long currentMillis = millis();
    if (currentMillis - previousWiFiCheck >= WIFI_CHECK_INTERVAL) {
        previousWiFiCheck = currentMillis;

        if (WiFi.status() != WL_CONNECTED && !isAPMode()) {
            Serial.println("WiFi disconnected, attempting to reconnect...");
            bool reconnected = reconnectWiFi();
            if (reconnected) {
                lastWiFiStatus = true;
            } else {
                lastWiFiStatus = false;
            }
        } else if (WiFi.status() == WL_CONNECTED && !lastWiFiStatus) {
            Serial.println("WiFi connected.");
            lastWiFiStatus = true;
        } else if (isAPMode() && WiFi.status() == WL_CONNECTED) {
            // Handle unexpected scenario where connected to WiFi while in AP mode
            Serial.println("Connected to WiFi while in AP mode. Stopping AP mode.");
            bool destroyed = destroyAP();
            if (destroyed) {
                Serial.println("AP mode successfully stopped.");
            } else {
                Serial.println("Failed to stop Access Point.");
            }
        }
    }

    delay(100);
}
