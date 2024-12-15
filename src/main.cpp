#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DNSServer.h>
#include "mic.h"
#include "config.h"
#include "wifisetup.h"
#include "requests.h"
#include "ui.h"
#include "rtc.h"
#include "battery.h"
#include "audio_manager.h"
#include "melodies.h"



// Variables to track Wi-Fi status
unsigned long previousWiFiCheck = 0;
bool lastWiFiStatus = false; // false: not connected, true: connected


// Task handles
TaskHandle_t micTaskHandle = NULL;
TaskHandle_t rtcTaskHandle = NULL;
TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t batteryTaskHandle = NULL;
TaskHandle_t uiTaskHandle = NULL;

// Mutex handle
SemaphoreHandle_t wifiMutex;

void micTask(void * pvParameters) {
    for (;;) {
        updateMic();
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

void rtcTask(void * pvParameters) {
    for (;;) {
        updateRTC();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void wifiTask(void * pvParameters) {
    for (;;) {
        xSemaphoreTake(wifiMutex, portMAX_DELAY);
        checkConnectionStatus(lastWiFiStatus, previousWiFiCheck);
        xSemaphoreGive(wifiMutex);
        if (client.available()) {
            sendPingWebSocket();
        };
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void batteryTask(void * pvParameters) {
    for (;;) {
        calculateBatteryLevelSprite();
        Serial.println("Free Heap: " + String(ESP.getFreeHeap()));


        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
}

void dnsTask(void * pvParameters) {
    for (;;) {
        if (isAPMode()) {
            dnsServer.processNextRequest();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void uiTask(void * pvParameters) {
    SpriteSheet currentSheet = getCurrentSpriteSheet(currentEmotion);
    int frameIndex = 0;

    for (;;) {
        switch (currentScreen) {
            case HOME:
                displayHomeScreen();
                 if (!isRecording) {
                    SpriteSheet newSheet = getCurrentSpriteSheet(currentEmotion);
                    if (newSheet.frames != currentSheet.frames) {
                        currentSheet = newSheet;
                        frameIndex = 0;
                    }
                    displayAnimation(currentSheet.frames[frameIndex]);
                    frameIndex = (frameIndex + 1) % currentSheet.frameCount;
                    delay(80); // Animation speed
                }
                break;
            case SETTINGS:
                displaySettingsScreen();
                break;
            case ERROR:
                displayErrorState("Error occurred!"); // Replace with your error message
                break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void setup(void) {
    Serial.begin(9600);
    while (!Serial);

    auto cfg = M5.config();
    M5.begin(cfg);

    // Play startup sound with proper resource management
    if (!AudioManager::getInstance()->playStartupSound()) {
        Serial.println("Failed to play startup sound");
    }
    
    delay(100); // Safety delay before mic init

    initScreen();

    if (!loadConfig()) {
        Serial.println("Failed to load configuration. Using default settings.");
    };

    bool connected = initWiFi();
    lastWiFiStatus = connected;


    initRTC();
    
    // Initialize microphone after speaker is fully cleaned up
    delay(100);  // Additional safety delay
    initMic();

    
    // Turn off the speaker to use the microphone

    // Create a mutex
    wifiMutex = xSemaphoreCreateMutex();

    // Create tasks
    xTaskCreatePinnedToCore(micTask, "Mic Task", 4096, NULL, 1, &micTaskHandle, 1);
    xTaskCreatePinnedToCore(rtcTask, "RTC Task", 2048, NULL, 1, &rtcTaskHandle, 1);
    xTaskCreatePinnedToCore(wifiTask, "WiFi Task", 2048, NULL, 1, &wifiTaskHandle, 0);
    xTaskCreatePinnedToCore(batteryTask, "Battery Task", 2048, NULL, 1, &batteryTaskHandle, 1);
    xTaskCreatePinnedToCore(dnsTask, "DNS Task", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(uiTask, "UI Task", 4096, NULL, 1, &uiTaskHandle, 1);
}

void loop() {
    M5.update();
    client.poll();

    // Example of how to switch screens based on events
    if (M5.BtnB.wasPressed()) {
        if (currentScreen == HOME) {
            switchScreen(SETTINGS);
        } else {
            switchScreen(HOME);
        } 
    }

    client.onMessage([](ws::WebsocketsClient &c, ws::WebsocketsMessage message) {
        Serial.println("Received WebSocket message:");
        Serial.println(message.data());

        if (message.data() == "celebration") {
            currentEmotion = "celebration";
            Serial.println("Switching to celebration animation");
            AudioManager::getInstance()->playSound(tone2, sizeof(tone2) / sizeof(Note));
        } else if (message.data() == "suspicious") {
            currentEmotion = "suspicious";
            Serial.println("Switching to suspicious animation");
            AudioManager::getInstance()->playSound(tone3, sizeof(tone3) / sizeof(Note));
        } else if (message.data() == "cute_smile") {
            currentEmotion = "cute_smile";
            Serial.println("Switching to cute smile animation");
        }
    });

    delay(10);
}