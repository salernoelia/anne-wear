#ifndef BATTERY_H
#define BATTERY_H

#include "M5StickCPlus2.h"

void calculateBatteryLevelSprite();

extern int batteryCapacityInMv;
extern int batteryLevelInPercent;


#endif // BATTERY_H
