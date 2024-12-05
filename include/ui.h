#ifndef UI_H
#define UI_H

#include <M5Unified.h>

void initScreen();
void displayHomeScreen();
void animateAudioWave(
    int16_t* data, 
    size_t record_length, 
    size_t record_samplerate,  
    size_t record_number, 
    size_t shift, 
    int16_t* prev_y, 
    int16_t* prev_h, 
    size_t draw_record_idx, 
    size_t rec_record_idx,
    int16_t* rec_data
);

#endif // UI_H