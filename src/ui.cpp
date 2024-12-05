#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5Unified.h>

void initScreen() {

    M5.Display.startWrite();
    // if (M5.Display.width() > M5.Display.height())
    // {
    //     M5.Display.setRotation(M5.Display.getRotation() ^ 1);
    // }
    M5.Display.setCursor(0, 0);
    M5.Display.print("REC\n");
}