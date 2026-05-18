/**
 * @brief Header defining class handling the beeper
 */

#pragma once

#include <SDL2/SDL_audio.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "common.hpp"
#include "machine_config.hpp"

// Varying the number of buffers is a balance between improving the quality of the output but increasing the delay in
// output
constexpr uint32_t num_buffers = 4;
constexpr uint16_t frequency = 22050;
constexpr float beeper_gain = 0.10f;
constexpr float ay_gain = 0.60f;

/**
 * @brief Class describing the beeper
 * This class assumes the SDL audio is already initialized.
 */
class Beeper {
public:
    explicit Beeper(const MachineModel &_machine)
        : machine(_machine),
          num_clocks_per_sample(static_cast<uint32_t>(machine.cpu_frequency_hz / frequency) + 1),
          samples(static_cast<uint16_t>(num_clocks_per_sample * 4)) {
        for (auto &buffer : data) {
            buffer.resize(samples);
        }

        SDL_zero(audiospec);
        audiospec.freq = frequency;
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
            if (len != bpr->samples) {
                fprintf(stderr, "Reading just %d bytes instead of %d\n", len, bpr->samples);
            }
            std::memcpy(reinterpret_cast<void *>(stream), reinterpret_cast<void *>(bpr->data[bpr->buffer_read].data()),
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

    void clock(bool is_ear_on, bool is_mic_on, uint64_t clocks) { clock(is_ear_on, is_mic_on, 0, clocks); }

    void clock(bool is_ear_on, bool is_mic_on, int32_t ay_level, uint64_t clocks) {
        if (clocks > 0) {
            num_clocks += clocks;

            const uint32_t beeper_step = (is_ear_on && is_mic_on) ? 4 : (is_ear_on ? 2 : 0);
            if (beeper_step > 0) {
                value += beeper_step * static_cast<uint32_t>(clocks);
            }
            ay_value += static_cast<int64_t>(ay_level) * static_cast<int64_t>(clocks);

            if (num_clocks > num_clocks_per_sample) {
                const uint32_t clamped_value = (value > 0x7f) ? 0x7f : value;
                const uint32_t scaled_value = static_cast<uint32_t>(clamped_value * beeper_gain);
                const int32_t averaged_ay =
                    static_cast<int32_t>(ay_value / static_cast<int64_t>(num_clocks_per_sample));
                const int32_t scaled_ay = static_cast<int32_t>(static_cast<float>(averaged_ay) * ay_gain);
                const int32_t mixed_value = std::clamp(static_cast<int32_t>(scaled_value) + scaled_ay, -128, 127);
                SDL_LockAudioDevice(device);

                data[buffer_write][index++] = static_cast<char>(mixed_value);
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
                num_clocks -= num_clocks_per_sample;
                if (num_clocks > 0 && beeper_step > 0) {
                    value = beeper_step;
                } else {
                    value = 0;
                }
                ay_value = static_cast<int64_t>(ay_level) * static_cast<int64_t>(num_clocks);
            }
        }
    }

    uint32_t buffer_read = {0xffffffff};
    bool buffer_read_ready = {false};
    uint32_t buffer_write = {0};
    uint32_t index = {0};
    uint32_t value = {0};
    int64_t ay_value = {0};

    std::array<std::vector<char>, num_buffers> data;

private:
    MachineModel machine;
    uint64_t num_clocks = {0};
    uint32_t num_clocks_per_sample = {0};
    uint16_t samples = {0};

    SDL_AudioSpec audiospec;
    SDL_AudioDeviceID device = {0};
};
