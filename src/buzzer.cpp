#include <Arduino.h>
#include "M5StickCPlus2.h"
#include "M5Unified.h"
#include "buzzer.h"
#include "melodies.h"

float scaleFactor = 2.0;
MelodyState currentMelody = {nullptr, 0, 0, false};

void initBuzzer() {
    M5.Speaker.begin();
    M5.Speaker.setVolume(250);
}

void playMelody(const Note melody[]) {
    currentMelody.melody = melody;
    currentMelody.currentNote = 0;
    currentMelody.nextNoteTime = 0;
    currentMelody.isPlaying = true;
}

void updateMelody() {
    if (!currentMelody.isPlaying) return;
    
    unsigned long currentTime = millis();
    
    if (currentTime >= currentMelody.nextNoteTime) {
        if (currentMelody.melody[currentMelody.currentNote].frequency == 0) {
            // End of melody
            currentMelody.isPlaying = false;
            M5.Speaker.tone(0, 0); // Stop sound
            return;
        }

        float scaledFrequency = currentMelody.melody[currentMelody.currentNote].frequency * scaleFactor;
        uint32_t duration = currentMelody.melody[currentMelody.currentNote].duration;
        
        M5.Speaker.tone(scaledFrequency, duration);
        currentMelody.nextNoteTime = currentTime + duration;
        currentMelody.currentNote++;
    }
}