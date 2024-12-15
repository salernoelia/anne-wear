#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <M5Unified.h>

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
    ~AudioManager();
};

#endif