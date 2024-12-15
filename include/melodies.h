// melodies.h
#ifndef MELODIES_H
#define MELODIES_H

#include <stdint.h>

struct Note {
    float frequency;
    uint32_t duration;
};

// Melody definitions
const Note startupMelody[] = {
    {587.32, 124},
    {493.88, 124},
    {587.32, 124},
    {783.99, 124},
    {0.0, 0}  // Terminating note to signal the end
};

#endif // MELODIES_H