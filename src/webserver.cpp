// webserver.cpp
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "webserver.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Flag to indicate if webserver is started
static bool webserverStarted = false;

// GET ("/") -> Configuration Form
void handleRoot(AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><title>Configuration</title></head><body>";
    html += "<h2>Configure Device</h2>";
    html += "<form action='/configure' method='post'>";
    html += "WiFi SSID:<br><input type='text' name='ssid' value='" + config.ssid + "' required><br>";
    html += "WiFi Password:<br><input type='password' name='password' value='" + config.password + "' required><br>";
    html += "Device ID:<br><input type='text' name='deviceID' value='" + config.deviceID + "' required><br>";
    html += "IP Address:<br><input type='text' name='ipaddress' value='" + config.ipaddress.toString() + "' required><br><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form></body></html>";

    request->send(200, "text/html", html);
}

// POST ("/configure") -> Save Configuration
void handleConfig(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true) &&
        request->hasParam("deviceID", true) && request->hasParam("ipaddress", true)) {

        config.ssid = request->getParam("ssid", true)->value();
        config.password = request->getParam("password", true)->value();
        config.deviceID = request->getParam("deviceID", true)->value();
        String ipStr = request->getParam("ipaddress", true)->value();
        config.ipaddress.fromString(ipStr.c_str());

        if (saveConfig()) {
            request->send(200, "text/html", "<!DOCTYPE html><html><head><title>Success</title></head><body><h2>Configuration Saved. Rebooting...</h2></body></html>");
            delay(1000);
            ESP.restart();
            return;
        } else { 
            request->send(500, "text/plain", "Failed to save configuration");
            return;
        }
    }

    // Missing parameters
    request->send(400, "text/plain", "Bad Request: Missing Parameters");
}

// GET ("/status") -> Device Status
void handleStatus(AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(1024);
    doc["deviceID"] = config.deviceID;
    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
    doc["ipaddress"] = config.ipaddress.toString();

    String json;
    serializeJsonPretty(doc, json);
    request->send(200, "application/json", json);
}

void okHandler(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK");
}

// Setup Web Server
void setupWebServer() {
    if (webserverStarted) {
        Serial.println("Web server already started.");
        return;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/configure", HTTP_POST, handleConfig);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/anne-running-check", HTTP_GET, okHandler);

    server.begin();
    webserverStarted = true;
    Serial.println("Web server started on port 80");
}
