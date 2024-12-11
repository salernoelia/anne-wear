#include "M5StickCPlus2.h"
#include "battery.h"

int batteryLevelInPercent;
int batteryCapacityInMv = 4200;

void batteryLevel() {
    batteryLevelInPercent = StickCP2.Power.getBatteryLevel();
}

void calculateBatteryLevelSprite() {
    batteryLevel();
    if (batteryLevelInPercent > 100) {
        // 100%
    } else if (batteryLevelInPercent > 75) {
        // 75%
    } else if (batteryLevelInPercent > 50) {
        // 50%
    } else if (batteryLevelInPercent > 25) {
        // 25%
    } else {
        // 0%
    }
}