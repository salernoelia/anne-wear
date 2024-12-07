// wifisetup.h

#ifndef WIFISETUP_H
#define WIFISETUP_H

#include <DNSServer.h>

bool initWiFi();
bool reconnectWiFi();
bool startAP();
bool destroyAP();
bool isAPMode();
void checkConnectionStatus(bool &lastWiFiStatus, unsigned long &previousWiFiCheck);

extern DNSServer dnsServer;

#endif // WIFISETUP_H
