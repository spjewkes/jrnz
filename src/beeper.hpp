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

constexpr uint16_t samples = 1024;
constexpr uint32_t num_buffers = 16;
constexpr uint32_t num_clocks_per_sample = 160;

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
        Beeper *bpr = reinterpret_cast<Beeper *>(userdata);
        if (bpr->buffer_read_ready) {
            if (len != samples) {
                fprintf(stderr, "Reading just %d bytes instead of %d\n", len, samples);
            }
            std::memcpy(reinterpret_cast<void *>(stream), reinterpret_cast<void *>(&bpr->data[bpr->buffer_read][0]),
                        len);
            bpr->buffer_read = (bpr->buffer_read + 1) % num_buffers;
            if (bpr->buffer_read == bpr->buffer_write) {
                // If we have caught up with the write buffer then we should stop reading until told to do so
                bpr->buffer_read_ready = false;
            }
        } else {
            std::memset(reinterpret_cast<void *>(stream), 0x0, len);
        }
    }

    void clock(bool is_on, uint64_t clocks) {
        if (clocks > 0) {
            num_clocks += clocks;

            if (!is_on) {
                value += 3;
            }

            if (num_clocks > num_clocks_per_sample) {
                if (value > 0x7f) {
                    value = 0x7f;
                }
                SDL_LockAudioDevice(device);

                data[buffer_write][index++] = value;
                if (index >= samples) {
                    // Reached the end of the current data buffer
                    // Move on to next buffer and mark previous buffer as ready to read
                    index = 0;
                    if (!buffer_read_ready) {
                        // If the buffer were not being read from then mark buffer as read ready
                        // before moving along to the next one
                        buffer_read = buffer_write;
                        buffer_read_ready = true;
                    }
                    buffer_write = (buffer_write + 1) % num_buffers;
                }
                SDL_UnlockAudioDevice(device);
                value = 0;
                num_clocks = 0;
            }
        }
    }

    uint32_t buffer_read = {0xffffffff};
    bool buffer_read_ready = {false};
    uint32_t buffer_write = {0};
    uint32_t index = {0};
    uint32_t value = {0};

    std::array<std::array<char, samples>, num_buffers> data;

private:
    uint64_t num_clocks = {0};

    SDL_AudioSpec audiospec;
    SDL_AudioDeviceID device;
};
