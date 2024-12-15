// buzzer.h
#ifndef BUZZER_H
#define BUZZER_H

#include <M5Unified.h>
#include "melodies.h"

struct MelodyState {
    const Note* melody;
    int currentNote;
    unsigned long nextNoteTime;
    bool isPlaying;
};

extern MelodyState currentMelody;

void playMelody(const Note melody[]);
void updateMelody();
void initBuzzer();

#endif // BUZZER_H
