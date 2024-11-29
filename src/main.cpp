// main.cpp
#include <Arduino.h>
#include <M5Unified.h>
#include "config.h"
#include "webserver.h"
#include <WiFi.h>





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

    // Connect to WiFi using loaded configuration
    Serial.print("Connecting to WiFi SSID: ");
    Serial.println(config.ssid);
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(2); // Ensure this is supported by your LCD library
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); 
    M5.Lcd.setCursor(0, 0); 

    Serial.print("Connecting to WiFi");
    M5.Lcd.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        M5.Lcd.print(".");
    }
    Serial.println();
    Serial.print("Connected to WiFi. IP address: ");
    Serial.println(WiFi.localIP());

    // Update config.ipaddress with the assigned IP
    config.ipaddress = WiFi.localIP();
    if (!saveConfig()) {
        Serial.println("Failed to save updated IP address to config.");
    }

    // Display configuration on LCD
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Device ID: " + config.deviceID);
    M5.Lcd.println("SSID: " + config.ssid);
    M5.Lcd.println("Password: " + config.password);
    M5.Lcd.println("IP Address: " + config.ipaddress.toString());

    // Print configuration to Serial
    Serial.println("Device ID: " + config.deviceID);
    Serial.println("SSID: " + config.ssid);
    Serial.println("Password: " + config.password);
    Serial.println("IP Address: " + config.ipaddress.toString());

    // Setup web server after WiFi is connected
    setupWebServer();
}

void loop() {
    M5.update(); // Handle background tasks
    delay(1000); // Wait for 1 second
}
