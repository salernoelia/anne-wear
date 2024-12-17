#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <M5Unified.h>


struct Note {
    float frequency;
    int duration;
};

class AudioManager {
private:
    static AudioManager* instance;
    bool speakerInitialized = false;
    
    AudioManager() {}
    
public:
    static AudioManager* getInstance();
    bool initSpeaker();
    void cleanupSpeaker();
    bool playStartupSound();
    bool playSound(const Note* notes, size_t length);
    bool playRawSound(const uint8_t* data, size_t length);
    ~AudioManager();
};

#endif