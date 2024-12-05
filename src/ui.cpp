#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5Unified.h>
#include "rtc.h"

bool needsScreenClear = false;

void initScreen() {

    M5.Display.startWrite();
    M5.Display.setRotation(1);
    M5.Display.setCursor(0, 0);
    M5.Display.print("REC\n");
    M5.Display.setTextSize(1.5);
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
                Serial.println(w);
                if (w > record_length - 1) { w = record_length - 1; }
                for (int32_t x = 0; x < w; ++x)
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

                if (++draw_record_idx >= record_number) { draw_record_idx = 0; }
                if (++rec_record_idx >= record_number) { rec_record_idx = 0; }
}

void displayHomeScreen() {
    if (needsScreenClear == true) {
        M5.Display.clear();
        needsScreenClear = false;
    }
    M5.Display.setCursor(0, 0);
    M5.Display.print("Home Screen");
    M5.Display.setCursor(0, 20);
    M5.Display.drawRect(0, 20, 240, 40);
    M5.Display.print(currentTime);
    M5.Display.display();

}