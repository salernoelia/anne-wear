#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5Unified.h>
#include "mic.h"
#include "rtc.h"
#include "battery.h"
#include "sprites/anne_logo.h"
#include "sprites/cute_smile.h"
#include "ui.h"



bool needsScreenClear = false;
String currentEmotion = "cute_smile";

SpriteSheet getCurrentSpriteSheet(const String& emotion) {
    if (emotion == "cute_smile") {
        return SpriteSheet{ cute_smile_sheet, cute_smile_frameCount };
    }
    // Add more conditions for other emotions
    else {
        return SpriteSheet{ cute_smile_sheet, cute_smile_frameCount };
    }
}



void displayAnimation(const uint16_t* frame) {
    // Clear the previous sprite with rect
    // Calculate half dimensions for centering
    int halfWidth = 96 / 2;
    int halfHeight = 96 / 2;

    M5.Display.drawRect(M5.Display.width() / 2 - halfWidth, M5.Display.height() / 2 - halfHeight, 96, 96, TFT_BLACK);
    M5.Display.pushImage(M5.Display.width() / 2 - halfWidth, M5.Display.height() / 2 - halfHeight, 96, 96, frame);
   

    // M5.Display.display();
}


void initScreen() {
    M5.Display.startWrite();
    M5.Display.setRotation(1);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(1.5);
    M5.Display.pushImage(M5.Display.width()/2-32, M5.Display.height()/2 -32, 64, 64, anne_logo);
    needsScreenClear = true;

}

void animateAudioWave (
   int16_t* data, 
   size_t record_length, 
   size_t record_samplerate,  
   size_t record_number, 
   size_t shift, 
   int16_t* prev_y, 
   int16_t* prev_h, 
   size_t draw_record_idx, 
   size_t rec_record_idx, 
   int16_t* rec_data
   ) {
     data = &rec_data[draw_record_idx * record_length];
                if (needsScreenClear == false) {
                    needsScreenClear = true;
                }

                M5.Display.setCursor(0, 0);

                int32_t w = M5.Display.width();
                if (w > record_length - 1) { 
                    if (record_length - 1 >= 240) {
                        w = 240;  
                    } else {
                        w = record_length - 1;  // Otherwise, set to the maximum allowed
                    }
                }
                for (int32_t x = 20; x < w; ++x)
                {
                    M5.Display.writeFastVLine(x, prev_y[x], prev_h[x], TFT_BLACK);
                    int32_t y1 = (data[x] >> shift);
                    int32_t y2 = (data[x + 1] >> shift);
                    if (y1 > y2)
                    {
                        int32_t tmp = y1;
                        y1 = y2;
                        y2 = tmp;
                    }
                    int32_t y = (M5.Display.height() >> 1) + y1;
                    int32_t h = (M5.Display.height() >> 1) + y2 + 1 - y;
                    prev_y[x] = y;
                    prev_h[x] = h;

                    M5.Display.writeFastVLine(x, y, h, TFT_WHITE);
                }
                M5.Display.display();


}

void displayHomeScreen() {
    

    if (needsScreenClear == true) {
        M5.Display.clear();
        needsScreenClear = false;
    }

    if (isRecording){
        return;
    }

    calculateBatteryLevelSprite();

    M5.Display.setCursor(0, 0);
    M5.Display.setColor(TFT_DARKGRAY);
    M5.Display.setColor(TFT_WHITE);

    M5.Display.print(currentTime);
    M5.Display.setCursor(0, 20);

    M5.Display.setCursor(M5.Display.width()-42, 0);
    M5.Display.print(batteryLevelInPercent);
    M5.Display.print("%");

    // M5.Display.pushImage(M5.Display.width()/2-48, M5.Display.height()/2 -48, 96, 96, cute_smile1);

    M5.Display.display();

}   

void displayErrorState(const String& errorMessage)
{
    M5.Display.clear();
    M5.Display.setCursor(10, 10);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println(errorMessage);
    // Optionally, add more details or a retry mechanism
}
