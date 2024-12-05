// mic.h
#ifndef MIC_H
#define MIC_H

#include <M5Unified.h>

void initMic();
void updateMic();
void sendAudioData();

constexpr size_t record_number = 256;
constexpr size_t record_length = 200;
static constexpr const size_t record_size = record_number * record_length;
constexpr size_t record_samplerate = 16000;

extern int16_t* rec_data;

#endif // MIC_H
