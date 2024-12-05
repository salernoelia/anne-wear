#include "M5StickCPlus2.h"
#include "battery.h"

int batteryLevelinMV;

void batteryLevel() {
    batteryLevelinMV = StickCP2.Power.getBatteryVoltage();
}