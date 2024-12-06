// wifisetup.cpp
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "config.h"
#include "wifisetup.h"
#include "M5Unified.h"
#include <DNSServer.h>
#include "webserver.h"
#include <mic.h>

// Constants
const unsigned long WIFI_CONNECTION_TIMEOUT = 15000;
const int MAX_RETRIES = 5;

// Variables
static bool isAP = false;
static int retryCount = 0;

// Interval for checking WiFi status (in milliseconds)
const unsigned long WIFI_CHECK_INTERVAL = 15000;

// Fixed AP IP configuration (Using a different subnet to avoid conflicts)
const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

DNSServer dnsServer;


const byte DNS_PORT = 53;

// Hostname for mDNS
const char* mdnsHostname = "anne-wear";

/**
 * @brief Initialize WiFi in Station (STA) mode.
 *
 * Attempts to connect to the configured WiFi network.
 * If successful, sets up mDNS and the web server.
 * If unsuccessful within the timeout, starts AP mode.
 *
 * @return true if connected to WiFi, false otherwise.
 */
bool initWiFi() {
    // Display initial status on LCD
    M5.Display.clear();
    M5.Display.setRotation(1);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.println("Connecting to WiFi...");

    // Attempt to connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);

    Serial.println(config.ssid);

    M5.Display.print("Connecting to: ");
    M5.Display.println(config.ssid);

    M5.Display.clear();

    unsigned long startAttemptTime = millis();

    // Try to connect until timeout
    while (WiFi.status() != WL_CONNECTED && 
        millis() - startAttemptTime < WIFI_CONNECTION_TIMEOUT) {
        if (!isAP) {
            Serial.print(".");
            M5.Display.print("."); // Update LCD with connection attempts
        }
        delay(500);
    }

    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("Connected to WiFi. IP address: ");
        Serial.println(WiFi.localIP());

        // Update config.ipaddress with the assigned IP
        config.ipaddress = WiFi.localIP();
        if (!saveConfig()) {
            Serial.println("Failed to save updated IP address to config.");
        }

        // Initialize mDNS
        if (!MDNS.begin(mdnsHostname)) {
            Serial.println("Error setting up mDNS responder!");
        } else {
            Serial.println("mDNS responder started. You can now connect using " + String(mdnsHostname) + ".local");
        }

        // Display configuration on LCD
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(0, 0);
        M5.Display.println("WiFi Connected!");
        M5.Display.println("Device ID: " + config.deviceID);
        M5.Display.println("SSID: " + config.ssid);
        M5.Display.println("IP Address: " + config.ipaddress.toString());

        // Print configuration to Serial
        Serial.println("Device ID: " + config.deviceID);
        Serial.println("SSID: " + config.ssid);
        Serial.println("IP Address: " + config.ipaddress.toString());

        // Setup web server after WiFi is connected
        setupWebServer();
        return true;
    } else {
        Serial.println();
        Serial.println("Failed to connect to WiFi within timeout.");
        Serial.println("Starting Access Point...");


        config.ipaddress = AP_IP;

        // Start Access Point
        if (startAP()) {
            return false;
        } else {
            Serial.println("Failed to start Access Point.");
            return false;
        }
    }
}

/**
 * @brief Attempt to reconnect to the configured WiFi network.
 *
 * If reconnection fails, starts AP mode.
 *
 * @return true if reconnection was successful, false otherwise.
 */
bool reconnectWiFi() {
    if (isAP) {
        Serial.println("Attempting to reconnect to WiFi from AP mode...");
    } else {
        Serial.println("Attempting to reconnect to WiFi...");
    }

    // Disconnect from current WiFi
    WiFi.disconnect();
    delay(1000);

    // Attempt to reconnect
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    unsigned long startAttemptTime = millis();

    // Try to connect until timeout
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttemptTime < WIFI_CONNECTION_TIMEOUT) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("Reconnected to WiFi. IP address: ");
        Serial.println(WiFi.localIP());

        // Update config.ipaddress with the assigned IP
        config.ipaddress = WiFi.localIP();
        if (!saveConfig()) {
            Serial.println("Failed to save updated IP address to config.");
        }

        // Initialize mDNS
        if (!MDNS.begin(mdnsHostname)) {
            Serial.println("Error setting up mDNS responder!");
        } else {
            Serial.println("mDNS responder started. You can now connect using " + String(mdnsHostname) + ".local");
        }

        // Update LCD
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(0, 0);
        M5.Display.println("WiFi Connected!");
        M5.Display.println("Device ID: " + config.deviceID);
        M5.Display.println("SSID: " + config.ssid);
        M5.Display.println("IP Address: " + config.ipaddress.toString());

        // Restart web server
        setupWebServer();

        // If in AP mode, destroy AP
        if (isAP) {
            if (destroyAP()) {
                Serial.println("AP mode successfully stopped.");
            } else {
                Serial.println("Failed to stop Access Point.");
            }
        }

        return true;
    } else {
        Serial.println();
        Serial.println("Failed to reconnect to WiFi.");

        // If not already in AP mode, start AP
        if (!isAP) {
            Serial.println("Starting Access Point...");
            if (startAP()) {
                return false;
            } else {
                Serial.println("Failed to start Access Point.");
                return false;
            }
        }

        return false;
    }
}

/**
 * @brief Start the device in Access Point (AP) mode with a fixed IP.
 *
 * Configures the AP with a fixed IP address and starts the web server.
 *
 * @return true if AP started successfully, false otherwise.
 */
bool startAP() {
    if (isAP) {
        Serial.println("AP is already running.");
        return true;
    }

    // Set fixed IP configuration for AP
    if (!WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET)) {
        Serial.println("AP Config failed.");
        return false;
    }

    // Start softAP with fixed IP
    bool apStarted = WiFi.softAP(config.apSSID.c_str(), config.apPassword.c_str());

    if (apStarted) {
        isAP = true;
        Serial.println("Access Point started.");
        Serial.print("AP SSID: ");
        Serial.println(config.apSSID);
        Serial.print("AP Password: ");
        Serial.println(config.apPassword);
        Serial.print("AP IP Address: ");
        Serial.println(WiFi.softAPIP());

        // Initialize mDNS for AP mode
        if (!MDNS.begin(mdnsHostname)) {
            Serial.println("Error setting up mDNS responder in AP mode!");
        } else {
            Serial.println("mDNS responder started in AP mode. Connect using " + String(mdnsHostname) + ".local");
        }

        // Start DNS Server to redirect all domains to AP_IP
        dnsServer.start(DNS_PORT, "*", AP_IP);

        if (!MDNS.addService("http", "tcp", 80)) {
            Serial.println("Failed to add mDNS service for HTTP in AP mode.");
        }



        // Display AP details on LCD
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(0, 0);
        M5.Display.qrcode("http://192.168.4.1/", 52, 0,
                  135);
        // Setup web server for AP mode
        setupWebServer();

        Serial.println("Captive Portal started at: http://" + AP_IP.toString());
        return true;
    } else {
        Serial.println("Failed to start Access Point.");
        return false;
    }
}

/**
 * @brief Stop the Access Point (AP) mode.
 *
 * Disconnects the AP and updates the system state.
 *
 * @return true if AP was stopped successfully, false otherwise.
 */
bool destroyAP() {
    if (!isAP) {
        Serial.println("AP is not running.");
        return true; // Not running, considered as success
    }

    // Set WiFi mode to WIFI_STA to ensure AP is disabled
    WiFi.mode(WIFI_STA);
    delay(1000); // Allow time for mode switch

    // Attempt to stop AP
    bool apStopped = WiFi.softAPdisconnect(true);
    if (apStopped) {
        Serial.println("Access Point stopped.");

        // Stop mDNS in AP mode
        MDNS.end();

        isAP = false;

        // Clear or update LCD as needed
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(0, 0);
        M5.Display.println("WiFi Disconnected");
        M5.Display.println("Attempting to reconnect...");
        return true;
    } else {
        Serial.println("Failed to stop Access Point.");

        // Attempt to force stop AP by setting mode to WIFI_STA again
        WiFi.mode(WIFI_STA);
        delay(1000); // Allow time for mode switch

        // Check if AP is stopped
        if (WiFi.getMode() == WIFI_STA && WiFi.softAPgetStationNum() == 0) {
            isAP = false;
            Serial.println("Access Point forcefully stopped by switching to STA mode.");

            // Stop mDNS if it was started in AP mode
            MDNS.end();

            // Clear or update LCD as needed
            M5.Display.fillScreen(TFT_BLACK);
            M5.Display.setCursor(0, 0);
            M5.Display.println("WiFi Disconnected");
            M5.Display.println("Attempting to reconnect...");

            return true;
        } else {
            Serial.println("Still unable to stop Access Point.");
            return false;
        }
    }
}

/**
 * @brief Check if the device is currently in Access Point (AP) mode.
 *
 * @return true if in AP mode, false otherwise.
 */
bool isAPMode() {
    return isAP;
}


void checkConnectionStatus(bool &lastWiFiStatus, unsigned long &previousWiFiCheck) {
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
                Serial.println("Wifi is connected:" + WL_CONNECTED);
            }
        }
    }
}