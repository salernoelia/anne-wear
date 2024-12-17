#include "audio_manager.h"
#include "ui.h"
#include "melodies.h"

AudioManager* AudioManager::instance = nullptr;

AudioManager* AudioManager::getInstance() {
    if (instance == nullptr) {
        instance = new AudioManager();
    }
    return instance;
}

bool AudioManager::initSpeaker() {
    if (!speakerInitialized) {
        M5.Speaker.stop();
        M5.Speaker.end();
        delay(100);
        speakerInitialized = M5.Speaker.begin();
        M5.Speaker.setVolume(255);
    }
    return speakerInitialized;
}

void AudioManager::cleanupSpeaker() {
    if (speakerInitialized) {
        M5.Speaker.stop();
        M5.Speaker.end();
        delay(100);
        speakerInitialized = false;
    }
}

bool AudioManager::playStartupSound() {
    if (!initSpeaker()) return false;
    
    const float notes[] = {587.32, 493.88, 587.32, 783.99};
    const int durations[] = {124, 124, 124, 124};
    
    for (int i = 0; i < 4; i++) {
        M5.Speaker.tone(notes[i], durations[i]);
        delay(durations[i] + 10);
    }
    
    cleanupSpeaker();
    return true;
}

bool AudioManager::playSound(const Note* notes, size_t length) {
    if (!initSpeaker()) return false;

    for (size_t i = 0; i < length; i++) {
        M5.Speaker.tone(notes[i].frequency * 2, notes[i].duration);
        delay(notes[i].duration + 10);
    }

    cleanupSpeaker();
    return true;
}

void AudioManager::playTone(float frequency, int duration) {
    if (!initSpeaker()) return;
    M5.Speaker.tone(frequency, duration);
    delay(duration + 10);
    cleanupSpeaker();
}

AudioManager::~AudioManager() {
    cleanupSpeaker();
}