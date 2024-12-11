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
        vTaskDelay(10 / portTICK_PERIOD_MS);
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
    // Initialize with the current emotion's sprite sheet
    SpriteSheet currentSheet = getCurrentSpriteSheet(currentEmotion);
    int frameIndex = 0;

    for (;;) {
        if (!isRecording) {
            // Check if the emotion has changed
            SpriteSheet newSheet = getCurrentSpriteSheet(currentEmotion);
            if (newSheet.frames != currentSheet.frames) {
                currentSheet = newSheet;
                frameIndex = 0; // Reset to the first frame of the new emotion
            }

            // Display the current frame
            displayAnimation(currentSheet.frames[frameIndex]); // Specify the desired position

            // Move to the next frame, looping back if necessary
            frameIndex = (frameIndex + 1) % currentSheet.frameCount;

            // Control the animation speed (adjust delay as needed)
            delay(800); // Delay in milliseconds
        }

        // Short delay to prevent task from hogging CPU (adjust as needed)
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void setup(void)
{
    // Initialize Serial for debugging
    Serial.begin(9600);
    while (!Serial); // Wait for Serial to initialize

    // Initialize M5Unified with default configuration
    auto cfg = M5.config();
    M5.begin(cfg);

    initScreen();

    if (!loadConfig()) {
        Serial.println("Failed to load configuration. Using default settings.");
    };

    bool connected = initWiFi();
    lastWiFiStatus = connected;

    M5.Speaker.end();

    initRTC();
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
    delay(10);
}
