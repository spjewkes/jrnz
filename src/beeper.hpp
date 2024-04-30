/**
 * @brief Header defining class handling the beeper
 */

#pragma once

#include <SDL2/SDL_audio.h>

#include <fstream>
#include <vector>

#include "common.hpp"

constexpr uint16_t samples = 512;

/**
 * @brief Class describing the beeper
 * This class assumes the SDL audio is already initialized.
 */
class Beeper {
public:
    Beeper() {
        SDL_zero(audiospec);
        audiospec.freq = 22050;
        audiospec.format = AUDIO_S8;
        audiospec.channels = 1;
        audiospec.samples = samples;
        audiospec.callback = nullptr;

        device = SDL_OpenAudioDevice(nullptr, 0, &audiospec, nullptr, 0);
        if (device == 0) {
            std::cerr << "Failed to initialize the audio device\n";
        } else {
            SDL_PauseAudioDevice(device, 0);
            data.push_back(0.0f);
            std::cout << "Audio initialized\n";
        }
    }
    virtual ~Beeper() {
        if (device != 0) {
            SDL_CloseAudioDevice(device);
        }
    }

    void clock(bool is_on, uint64_t clocks) {
        if (clocks > 0) {
            num_clocks += clocks;

            if (num_clocks > 159) {
                num_clocks = 0;
                index++;
                data.push_back(0x0);
            }

            if (is_on) {
                data[index] = 0x7f;
            }

            counter++;
        }

        // if (counter > 800) {
        if (index >= (samples - 1)) {
            // currently silent
            SDL_QueueAudio(device, data.data(), data.size());
            data.clear();
            data.push_back(0.0f);
            index = 0;
            num_clocks = 0;
            counter = 0;
        }
    }

private:
    uint32_t index = {0};
    uint64_t num_clocks = {0};
    uint32_t counter = {0};

    std::vector<char> data;

    SDL_AudioSpec audiospec;
    SDL_AudioDeviceID device;
};
