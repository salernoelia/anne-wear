#ifndef UI_H
#define UI_H

#include <M5Unified.h>

enum Screen {
    HOME,
    SETTINGS,
    ERROR
};

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
void displayErrorState(const String& errorMessage);
void switchScreen(Screen screen); // Function declaration
void displaySettingsScreen();

extern Screen currentScreen; // Declare the global variable
extern bool needsScreenClear;
extern String currentEmotion;

#endif // UI_H

