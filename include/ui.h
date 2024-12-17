#ifndef UI_H
#define UI_H

#include <M5Unified.h>

#include "requests.h" 

enum Screen {
    HOME,
    ACTIVITIES,
    COMPOSER,
    SETTINGS,
    ERROR
};

struct SpriteSheet {
    const uint16_t** frames; // Pointer to the array of frame pointers in PROGMEM
    int frameCount;          // Number of frames in the sprite sheet
};

extern std::vector<Task> tasks;

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
void switchScreen(Screen screen); 
void displaySettingsScreen();
void handleComposerButtons();
void displayActivitiesScreen();
void displayComposerScreen();

extern Screen currentScreen;
extern bool needsScreenClear;
extern String currentEmotion;

extern const int MAX_NOTES;
extern const int NOTE_HEIGHTS[];
extern int compositionIndex;
extern int currentNoteIndex;
extern int composition[4];
extern bool compositionReplayed;


#endif // UI_H

