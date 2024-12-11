#ifndef UI_H
#define UI_H

#include <M5Unified.h>

struct SpriteSheet {
    const uint16_t** frames; // Pointer to the array of frame pointers in PROGMEM
    int frameCount;          // Number of frames in the sprite sheet
};


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

void displayAnimation(const uint16_t* emotion);
SpriteSheet getCurrentSpriteSheet(const String& emotion);




extern bool needsScreenClear;
extern const uint16_t* spriteSheets[];
extern const size_t spriteSheetCount;
extern const uint16_t* currentSprite;

extern String currentEmotion;




#endif // UI_H
