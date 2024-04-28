/**
 * @brief Header defining class handling the beeper
 */

#pragma once

#include <SDL2/SDL_audio.h>

#include <vector>

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
        audiospec.format = AUDIO_U16;
        audiospec.channels = 1;
        audiospec.samples = 4096;
        audiospec.callback = nullptr;

        device = SDL_OpenAudioDevice(nullptr, 0, &audiospec, nullptr, 0);
        if (device == 0) {
            std::cerr << "Failed to initialize the audio device\n";
        } else {
            SDL_PauseAudio(0);
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

        if (clocks > 0) {
            uint16_t value = 0x0fff;
            if (!is_on) {
                value = 0;
            }

            // for (auto i = 0; i < 72; i++) {
            for (auto i = 0; i < 72; i++) {
                data.push_back(value);
            }
            counter++;
        }

        if (counter > 800) {
            // for (auto val : data) {
            // 	std::cout << val << "\n";
            // }

            SDL_QueueAudio(device, data.data(), data.size());
            data.clear();
            counter = 0;
        }
    }

private:
    uint32_t counter = {0};

    std::vector<uint16_t> data;

    SDL_AudioSpec audiospec;
    SDL_AudioDeviceID device;
};
