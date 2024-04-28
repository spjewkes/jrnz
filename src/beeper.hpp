/**
 * @brief Header defining class handling the beeper
 */

#pragma once

#include <SDL2/SDL_audio.h>

#include "common.hpp"

/**
 * @brief Class describing the beeper
 * This class assumes the SDL audio is already initialized.
 */
class Beeper {
public:
    Beeper() {
        SDL_zero(audiospec);
        audiospec.freq = 48000;
        audiospec.format = AUDIO_F32;
        audiospec.channels = 1;
        audiospec.samples = 4096;
        audiospec.callback = nullptr;

        device = SDL_OpenAudioDevice(nullptr, 0, &audiospec, nullptr, 0);
        if (device == 0) {
            std::cerr << "Failed to initialize the audio device\n";
        } else {
            std::cout << "Audio initialized\n";
        }
    }
    virtual ~Beeper() {
        if (device != 0) {
            SDL_CloseAudioDevice(device);
        }
    }

    void clock(bool is_on, uint64_t clocks) {
        UNUSED(is_on);
        UNUSED(clocks);
    }

private:
    SDL_AudioSpec audiospec;
    SDL_AudioDeviceID device;
};
