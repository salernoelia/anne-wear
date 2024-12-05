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

// Function to set CORS headers on a response
void setCorsHeaders(AsyncWebServerResponse *response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

// GET ("/") -> Configuration Form
void handleRoot(AsyncWebServerRequest *request) {
    // Handle preflight OPTIONS request
    if (request->method() == HTTP_OPTIONS) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
        setCorsHeaders(response);
        request->send(response);
        return;
    }

    String html = "<!DOCTYPE html><html><head><title>Configuration</title></head><body>";
    html += "<h2>Configure Device</h2>";
    html += "IP Address:<br><h1> " + config.ipaddress.toString() + " </h1><br><br>";
    html += "<form action='/configure' method='post'>";
    html += "WiFi SSID:<br><input type='text' name='ssid' value='" + config.ssid + "' required><br>";
    html += "WiFi Password:<br><input type='password' name='password' value='" + config.password + "' required><br>";
    html += "Device Name:<br><input type='text' name='deviceName' value='" + config.deviceName + "' required><br>";
    html += "Device ID:<br><input type='text' name='deviceID' value='" + config.deviceID + "' required><br>";
    html += "User ID:<br><input type='text' name='userID' value='" + config.userID + "' required><br>";
    html += "Server URL:<br><input type='text' name='serverURL' value='" + config.serverURL + "'><br><br>";

    String languageOptionEn = (config.language == "en") ? "selected" : "";
    String languageOptionDe = (config.language == "de") ? "selected" : "";

    html += "<select name='language' id='language'>";
    html += "<option value='en' " + languageOptionEn + ">English</option>";
    html += "<option value='de' " + languageOptionDe + ">Deutsch</option>";
    html += "</select><br><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form></body></html>";

    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
    setCorsHeaders(response);
    request->send(response);
}

// POST ("/configure") -> Save Configuration
void handleConfig(AsyncWebServerRequest *request) {
    // Handle preflight OPTIONS request
    if (request->method() == HTTP_OPTIONS) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
        setCorsHeaders(response);
        request->send(response);
        return;
    }

    if (request->hasParam("ssid", true) && request->hasParam("password", true) &&
    // request->hasParam("deviceName", true) && request->hasParam("ipaddress", true) &&
    request->hasParam("userID", true) && request->hasParam("deviceID", true)) {

            config.ssid = request->getParam("ssid", true)->value();
            config.password = request->getParam("password", true)->value();
            config.deviceName = request->getParam("deviceName", true)->value();
            // String ipStr = request->getParam("ipaddress", true)->value();
            // config.ipaddress.fromString(ipStr.c_str());
            config.deviceID = request->getParam("deviceID", true)->value();
            config.userID = request->getParam("userID", true)->value();
            config.serverURL = request->getParam("serverURL", true)->value();
            if (request->hasParam("language", true)) {
                config.language = request->getParam("language", true)->value();
            } else {
                config.language = "en";
            }

        if (saveConfig()) {
            String successHtml = "<!DOCTYPE html><html><head><title>Success</title></head><body><h2>Configuration Saved. Rebooting...</h2></body></html>";
            AsyncWebServerResponse *response = request->beginResponse(200, "text/html", successHtml);
            setCorsHeaders(response);
            request->send(response);
            delay(1000);
            ESP.restart();
            return; 
        } else { 
            AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "Failed to save configuration");
            setCorsHeaders(response);
            request->send(response);
            return;
        }
    }

    // Missing parameters
    AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "Bad Request: Missing Parameters");
    setCorsHeaders(response);
    request->send(response);
}

// GET ("/ok") -> Simple Status Check
void handleOk(AsyncWebServerRequest *request) {
    // Handle preflight OPTIONS request
    if (request->method() == HTTP_OPTIONS) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
        setCorsHeaders(response);
        request->send(response);
        return;
    }

    // Return JSON response
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"ok\"}");
    setCorsHeaders(response);
    request->send(response);
}

// Setup Web Server
void setupWebServer() {
    if (webserverStarted) {
        Serial.println("Web server already started.");
        return;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/configure", HTTP_POST, handleConfig);
    server.on("/ok", HTTP_GET, handleOk);

    // Handle all OPTIONS requests for CORS preflight
    server.onNotFound([](AsyncWebServerRequest *request){
        if (request->method() == HTTP_OPTIONS){
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            request->send(response);
        }
        else {
            request->send(404, "text/plain", "Not Found");
        }
    });

    server.begin();
    webserverStarted = true;
    Serial.println("Web server started on port 80");
}
