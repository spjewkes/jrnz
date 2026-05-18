/**
 * @brief Simple AY-3-8912 sound chip model used by the 128K Spectrum.
 */

#pragma once

#include <array>
#include <cstdint>

#include "machine_config.hpp"

class AyChip {
public:
    explicit AyChip(const MachineModel &_machine) : machine(_machine) { reset(); }

    void reset();
    void select_register(uint8_t reg) { selected_register_value = reg & 0x0f; }
    void write_register(uint8_t reg, uint8_t value);
    void write_selected_register(uint8_t value);
    uint8_t read_selected_register() const { return registers[selected_register_value]; }
    uint8_t selected_register() const { return selected_register_value; }
    uint8_t register_value(uint8_t reg) const { return registers[reg & 0x0f]; }
    void restore_state(uint8_t selected_register, const std::array<uint8_t, 16> &restored_registers);

    void clock_cpu_tstate();
    int32_t output_level() const;

private:
    struct ToneChannel {
        uint32_t counter = {0};
        bool high = {true};
    };

    uint16_t tone_period(uint8_t channel) const;
    uint32_t tone_half_period(uint8_t channel) const;
    uint32_t noise_half_period() const;
    uint32_t envelope_period() const;
    uint8_t channel_volume(uint8_t channel) const;
    void clock_ay_tick();
    void clock_tone_channel(uint8_t channel);
    void clock_noise();
    void clock_envelope();
    void reset_envelope();
    static uint8_t sanitized_register_value(uint8_t reg, uint8_t value);

    MachineModel machine;
    std::array<uint8_t, 16> registers{};
    std::array<ToneChannel, 3> tone_channels{};
    uint8_t selected_register_value = {0};
    uint64_t ay_clock_accumulator = {0};
    uint32_t noise_counter = {0};
    uint32_t envelope_counter = {0};
    uint32_t envelope_step = {0};
    uint32_t noise_lfsr = {0x1ffff};
    bool noise_high = {true};
    bool envelope_holding = {false};
    bool envelope_direction_up = {false};
};
