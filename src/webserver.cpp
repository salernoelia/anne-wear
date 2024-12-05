// webserver.cpp
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "webserver.h"

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

    String html = "<!DOCTYPE html><html><head><title>Configuration</title>";
    html += "<style>";
    html += "body {";
    html += "    font-family: Arial, sans-serif;";
    html += "    background-color: #f2f2f2;";
    html += "    margin: 0;";
    html += "    padding: 0;";
    html += "}";
    html += ".container {";
    html += "    width: 50%;";
    html += "    margin: 50px auto;";
    html += "    background-color: #fff;";
    html += "    padding: 20px;";
    html += "    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);";
    html += "    border-radius: 8px;";
    html += "}";
    html += "h2 {";
    html += "    text-align: center;";
    html += "    color: #333;";
    html += "}";
    html += ".ip-address {";
    html += "    text-align: center;";
    html += "    margin-bottom: 20px;";
    html += "}";
    html += ".ip-address h1 {";
    html += "    color: #4CAF50;";
    html += "    margin: 10px 0;";
    html += "}";
    html += "form {";
    html += "    display: flex;";
    html += "    flex-direction: column;";
    html += "}";
    html += "label {";
    html += "    margin-bottom: 5px;";
    html += "    color: #555;";
    html += "}";
    html += "input[type='text'], input[type='password'], select {";
    html += "    padding: 10px;";
    html += "    margin-bottom: 15px;";
    html += "    border: 1px solid #ccc;";
    html += "    border-radius: 4px;";
    html += "    font-size: 16px;";
    html += "}";
    html += "input[type='submit'] {";
    html += "    padding: 10px;";
    html += "    background-color: #4CAF50;";
    html += "    color: white;";
    html += "    border: none;";
    html += "    border-radius: 4px;";
    html += "    cursor: pointer;";
    html += "    font-size: 16px;";
    html += "}";
    html += "input[type='submit']:hover {";
    html += "    background-color: #45a049;";
    html += "}";
    html += "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h2>Configure Device</h2>";
    html += "<div class='ip-address'>IP Address:<br><h1>" + config.ipaddress.toString() + "</h1></div>";
    html += "<form action='/configure' method='post'>";
    html += "<label for='ssid'>WiFi SSID:</label>";
    html += "<input type='text' id='ssid' name='ssid' value='" + config.ssid + "' required>";

    html += "<label for='password'>WiFi Password:</label>";
    html += "<input type='password' id='password' name='password' value='" + config.password + "' required>";

    html += "<label for='deviceName'>Device Name:</label>";
    html += "<input type='text' id='deviceName' name='deviceName' value='" + config.deviceName + "' required>";

    html += "<label for='deviceID'>Device ID:</label>";
    html += "<input type='text' id='deviceID' name='deviceID' value='" + config.deviceID + "' required>";

    html += "<label for='userID'>User ID:</label>";
    html += "<input type='text' id='userID' name='userID' value='" + config.userID + "' required>";

    html += "<label for='serverURL'>Server URL:</label>";
    html += "<input type='text' id='serverURL' name='serverURL' value='" + config.serverURL + "'>";

    String languageOptionEn = config.language.equals("en") ? "selected" : "";
    String languageOptionDe = config.language.equals("de") ? "selected" : "";

    html += "<label for='language'>Language:</label>";
    html += "<select name='language' id='language'>";
    html += "<option value='en' " + languageOptionEn + ">English</option>";
    html += "<option value='de' " + languageOptionDe + ">Deutsch</option>";
    html += "</select>";

    html += "<input type='submit' value='Save'>";
    html += "</form>";
    html += "</div>"; 
    html += "</body></html>";


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
