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
#include <typeinfo>
#include <string>

// Variables to track Wi-Fi status
unsigned long previousWiFiCheck = 0;
bool lastWiFiStatus = false; // false: not connected, true: connected


// Task handles
TaskHandle_t micTaskHandle = NULL;
TaskHandle_t rtcTaskHandle = NULL;
TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t batteryTaskHandle = NULL;
TaskHandle_t uiTaskHandle = NULL;
TaskHandle_t buttonsTaskHandle = NULL;
TaskHandle_t webSocketPollingTaskHandle = NULL;

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
        if (isAPMode()) {
            dnsServer.processNextRequest();
            if (M5.BtnA.wasClicked()) {
            reconnectWiFi();
            }
        }
        

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void batteryTask(void * pvParameters) {
    for (;;) {
        calculateBatteryLevelSprite();
        Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
        Serial.println("Free Stack: " + String(uxTaskGetStackHighWaterMark(NULL)));


        vTaskDelay(20000 / portTICK_PERIOD_MS);
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
            case ACTIVITIES:
                displayActivitiesScreen();
                break;
            case COMPOSER:
                displayComposerScreen();
                break;
            case ERROR:
                displayErrorState("Error occurred!"); // Replace with your error message
                break;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}



void setup(void) {
    Serial.begin(9600);
    while (!Serial);

    auto cfg = M5.config();
    M5.begin(cfg);

    if (!AudioManager::getInstance()->playStartupSound()) {
        Serial.println("Failed to play startup sound");
    }
    
    delay(100); 

    initScreen();

    if (!loadConfig()) {
        Serial.println("Failed to load configuration. Using default settings.");
    };

    bool connected = initWiFi();
    lastWiFiStatus = connected;


    initRTC();
    delay(100); 
    initMic();

    wifiMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(micTask, "Mic Task", 4096, NULL, 2, &micTaskHandle, 1);
    xTaskCreatePinnedToCore(rtcTask, "RTC Task", 2048, NULL, 1, &rtcTaskHandle, 1);
    xTaskCreatePinnedToCore(wifiTask, "WiFi Task", 2048, NULL, 2, &wifiTaskHandle, 0);
    xTaskCreatePinnedToCore(batteryTask, "Battery Task", 2048, NULL, 1, &batteryTaskHandle, 1);
    xTaskCreatePinnedToCore(uiTask, "UI Task", 4096, NULL, 1, &uiTaskHandle, 1);

    getUserTasksThatAreStillDue();
}

void loop() {
    M5.update();


    client.poll();
    handleComposerButtons();

    client.onMessage([](ws::WebsocketsClient &c, ws::WebsocketsMessage message) {
        Serial.println("Received WebSocket message:");
        Serial.println(message.data());
        if (message.data() == "celebration") {
            currentEmotion = "celebration";
            Serial.println("Switching to celebration animation");
            
        AudioManager::getInstance()->playSound(celebration, sizeof(celebration) / sizeof(Note));
        } else if (message.data() == "suspicious") {
            currentEmotion = "suspicious";
            Serial.println("Switching to suspicious animation");
            AudioManager::getInstance()->playSound(suspicious, sizeof(suspicious) / sizeof(Note));
        } else if (message.data() == "cute_smile") {
            currentEmotion = "cute_smile";
            Serial.println("Switching to cute smile animation");
            AudioManager::getInstance()->playSound(cute_smile, sizeof(cute_smile) / sizeof(Note));
        } else if (message.data() == "curiosity") {
            currentEmotion = "curiosity";
            Serial.println("Switching to curiosity animation");
            AudioManager::getInstance()->playSound(curiosity, sizeof(curiosity) / sizeof(Note));
        } else if (message.data() == "confused") {
            currentEmotion = "confused";
            Serial.println("Switching to confused animation");
            AudioManager::getInstance()->playSound(confused, sizeof(confused) / sizeof(Note));
        } else if (message.data() == "sleep") {
            currentEmotion = "sleeping";
            Serial.println("Switching to sleep animation");
            AudioManager::getInstance()->playSound(sleeping, sizeof(sleeping) / sizeof(Note));
        } else if (message.data() == "lucky_smile") {
            currentEmotion = "lucky_smile";
            Serial.println("Switching to lucky smile animation");
            AudioManager::getInstance()->playSound(lucky_smile, sizeof(lucky_smile) / sizeof(Note));
        } else if (message.data() == "surprised") {
            currentEmotion = "surprised";
            Serial.println("Switching to surprised animation");
            AudioManager::getInstance()->playSound(surprised, sizeof(surprised) / sizeof(Note));
        }
    });

   

     if (M5.BtnB.wasClicked()) {
            if (currentScreen == HOME) {
                needsScreenClear = true;
                getUserTasksThatAreStillDue();
                
                switchScreen(ACTIVITIES);
            } else if (currentScreen == ACTIVITIES) {
                needsScreenClear = true;
                switchScreen(COMPOSER);
            } else if (currentScreen == COMPOSER) {
                needsScreenClear = true;
                compositionIndex = 0;
                memset(composition, 0, sizeof(composition));
                needsScreenClear = true;
                compositionReplayed = false;
                switchScreen(SETTINGS);

            } else if (currentScreen == SETTINGS) {
                needsScreenClear = true;
                switchScreen(HOME);
            }
        }


    delay(10);
}