// wifisetup.h

#ifndef WIFISETUP_H
#define WIFISETUP_H

bool initWiFi();
bool reconnectWiFi();
bool startAP();
bool destroyAP();
bool isAPMode();
void checkConnectionStatus(bool &lastWiFiStatus, unsigned long &previousWiFiCheck);

#endif // WIFISETUP_H
