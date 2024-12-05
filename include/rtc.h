#ifndef RTC_H
#define RTC_H

#include <M5Unified.h>

void initRTC(void);
void updateRTC(void);

extern struct tm timeinfo;
extern struct String currentTime;

#endif // RTC_H
