#ifndef CONFIG_H
#define CONFIG_H

#include <IPAddress.h>
#include <Arduino.h>

// File path for configuration
extern const char* configPath;

// Default Configuration Structure Declaration
struct Config {
    String deviceID;
    String deviceName;
    String apSSID;
    String apPassword;

    String ssid;
    String password;
    IPAddress ipaddress;
};

// External declaration of the global config variable
extern Config config;

// Function prototypes
bool loadConfig();
bool saveConfig();

#endif // CONFIG_H
