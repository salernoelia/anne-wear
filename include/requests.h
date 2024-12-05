#ifndef REQUESTS_H
#define REQUESTS_H

#include "config.h"    // Include config.h to ensure config is available
#include <HTTPClient.h> // HTTPClient library for making HTTP requests
#include <ArduinoJson.h> // ArduinoJson for handling JSON data
#include <M5Unified.h>   // M5Unified for M5Stack display

// Function prototype
void sendAudioRequest(
    int16_t* rec_data,  // Audio data received
    size_t record_size  // Size of the audio data
);

#endif // REQUESTS_H
