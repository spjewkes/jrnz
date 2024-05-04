/**
 * @brief Header defining class handling the beeper
 */

#pragma once

#include <SDL2/SDL_audio.h>

#include <array>
#include <cmath>
#include <fstream>
#include <string>

#include "common.hpp"

constexpr uint16_t samples = 128;
constexpr size_t buffer_size = 65536;

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
        audiospec.callback = Beeper::audio_callback;
        audiospec.userdata = reinterpret_cast<void *>(this);

        device = SDL_OpenAudioDevice(nullptr, 0, &audiospec, nullptr, 0);
        if (device == 0) {
            std::cerr << "Failed to initialize the audio device\n";
        } else {
            SDL_PauseAudioDevice(device, 0);
            std::cout << "Audio initialized\n";
        }
    }
    virtual ~Beeper() {
        if (device != 0) {
            SDL_CloseAudioDevice(device);
        }
    }

    static void audio_callback(void *userdata, Uint8 *stream, int len) {
        std::memset(reinterpret_cast<void *>(stream), 0x0, len);

        Beeper *bpr = reinterpret_cast<Beeper *>(userdata);
        // if (bpr->read != bpr->write) {
        if (abs(static_cast<int>(bpr->read - bpr->write)) >= samples) {
            // some data to copy
            if (bpr->read < bpr->write) {
                int size = bpr->write - bpr->read;
                if (size > len) {
                    size = len;
                }
                std::memcpy(reinterpret_cast<void *>(stream), reinterpret_cast<void *>(&bpr->data[bpr->read]), size);
                bpr->read += size;
            } else {
                int size = buffer_size - 1 - bpr->read;
                if (size > len) {
                    size = len;
                }
                std::memcpy(reinterpret_cast<void *>(stream), reinterpret_cast<void *>(&bpr->data[bpr->read]), size);
                bpr->read = 0;
            }
        }
    }

    void clock(bool is_on, uint64_t clocks) {
        if (clocks > 0) {
            num_clocks += clocks;

            if (!is_on) {
                value += 3;
            }

            if (num_clocks > 159) {
                if (value > 0x7f) {
                    value = 0x7f;
                }
                SDL_LockAudioDevice(device);
                // Don't bounds check values just yet
                data[write++] = static_cast<char>(value);
                if (write == buffer_size) {
                    write = 0;
                }
                SDL_UnlockAudioDevice(device);
                value = 0;
                num_clocks = 0;
            }
        }
    }

    uint32_t read = {0};
    uint32_t write = {0};
    uint32_t value = {0};

    std::array<char, buffer_size> data = {0};

private:
    uint64_t num_clocks = {0};

    SDL_AudioSpec audiospec;
    SDL_AudioDeviceID device;
};
