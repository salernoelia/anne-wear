#include <Arduino.h>
#include <SPIFFS.h>
#include <memory>
#include <ArduinoJson.h>
#include "config.h"

// Define the file path for configuration
const char* configPath = "/config.json";

// Initialize the global config variable with default values
Config config = {
    "715e4fe4-e04a-406d-8282-8f92ddb464cc", // deviceID
    "anne",                                // deviceName
    "anne-wear",                           // apSSID
    "anne-wear",                           // apPassword
    "",                                     // ssid
    "",                                     // password
    IPAddress(192, 168, 1, 118)            // ipaddress
};

// Function to load configuration from SPIFFS
bool loadConfig() {

    if (!SPIFFS.begin(true)) { // Initialize SPIFFS
        Serial.println("Failed to mount SPIFFS");
        return false;
    }

    if (!SPIFFS.exists(configPath)) {
        Serial.println("Config file does not exist");
        return false;
    }

    File file = SPIFFS.open(configPath, "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return false;
    }


    size_t size = file.size();
    if (size > 2048) { // Increased size to accommodate new fields
        Serial.println("Config file size is too large");
        file.close();
        return false;
    }

    // Allocate buffer
    std::unique_ptr<char[]> buf(new char[size + 1]);
    file.readBytes(buf.get(), size);
    buf[size] = '\0';
    file.close();

    // Parse JSON
    DynamicJsonDocument doc(2048); // Increased size
    auto error = deserializeJson(doc, buf.get());
    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }

    // Assign values to config with checks
    if (doc.containsKey("deviceID")) {
        config.deviceID = doc["deviceID"].as<String>();
    }
    if (doc.containsKey("deviceName")) {
        config.deviceName = doc["deviceName"].as<String>();
    }
    if (doc.containsKey("apSSID")) {
        config.apSSID = doc["apSSID"].as<String>();
    }
    if (doc.containsKey("apPassword")) {
        config.apPassword = doc["apPassword"].as<String>();
    }
    if (doc.containsKey("ssid")) {
        config.ssid = doc["ssid"].as<String>();
    }
    if (doc.containsKey("password")) {
        config.password = doc["password"].as<String>();
    }
    if (doc.containsKey("ipaddress")) {
        String ipStr = doc["ipaddress"].as<String>();
        config.ipaddress.fromString(ipStr.c_str());
    }

    Serial.println("Configuration loaded:");
    serializeJsonPretty(doc, Serial);
    Serial.println();

    return true;
}

// Function to save configuration to SPIFFS
bool saveConfig() {
    if (!SPIFFS.begin(true)) { // Initialize SPIFFS
        Serial.println("Failed to mount SPIFFS");
        return false;
    }

    DynamicJsonDocument doc(2048); // Increased size
    doc["deviceID"] = config.deviceID;
    doc["deviceName"] = config.deviceName;
    doc["apSSID"] = config.apSSID;
    doc["apPassword"] = config.apPassword;
    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
    doc["ipaddress"] = config.ipaddress.toString();

    File file = SPIFFS.open(configPath, "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to config file");
        file.close();
        return false;
    }

    file.close();
    Serial.println("Configuration saved");
    return true;
}