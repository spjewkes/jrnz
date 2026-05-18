#include "ay.hpp"

#include <algorithm>

namespace {
constexpr std::array<uint8_t, 16> volume_table = {0, 1, 1, 2, 3, 4, 6, 8, 11, 15, 20, 27, 35, 46, 61, 81};
constexpr uint8_t mixer_tone_disable_bit = 0x01;
constexpr uint8_t mixer_noise_disable_bit = 0x08;
constexpr uint8_t envelope_enable_bit = 0x10;
constexpr uint8_t envelope_continue_bit = 0x08;
constexpr uint8_t envelope_attack_bit = 0x04;
constexpr uint8_t envelope_alternate_bit = 0x02;
constexpr uint8_t envelope_hold_bit = 0x01;
}  // namespace

void AyChip::reset() {
    registers.fill(0);
    tone_channels = {};
    selected_register_value = 0;
    ay_clock_accumulator = 0;
    noise_counter = 0;
    envelope_counter = 0;
    envelope_step = 0;
    noise_lfsr = 0x1ffff;
    noise_high = true;
    reset_envelope();
}

void AyChip::write_register(uint8_t reg, uint8_t value) {
    const uint8_t index = reg & 0x0f;
    const uint8_t sanitized_value = sanitized_register_value(index, value);
    if (registers[index] == sanitized_value) {
        return;
    }

    registers[index] = sanitized_value;
    if (index == 13) {
        reset_envelope();
    }
}

void AyChip::write_selected_register(uint8_t value) { write_register(selected_register_value, value); }

void AyChip::restore_state(uint8_t selected_register, const std::array<uint8_t, 16> &restored_registers) {
    selected_register_value = selected_register & 0x0f;
    for (uint8_t reg = 0; reg < registers.size(); ++reg) {
        registers[reg] = sanitized_register_value(reg, restored_registers[reg]);
    }
    reset_envelope();
}

void AyChip::clock_cpu_tstate() {
    if (machine.ay_frequency_hz == 0) {
        return;
    }

    ay_clock_accumulator += machine.ay_frequency_hz;
    while (ay_clock_accumulator >= machine.cpu_frequency_hz) {
        ay_clock_accumulator -= machine.cpu_frequency_hz;
        clock_ay_tick();
    }
}

int32_t AyChip::output_level() const {
    if (machine.ay_frequency_hz == 0) {
        return 0;
    }

    const uint8_t mixer = registers[7];
    int32_t level = 0;
    for (uint8_t channel = 0; channel < tone_channels.size(); ++channel) {
        const bool tone_enabled = (mixer & static_cast<uint8_t>(mixer_tone_disable_bit << channel)) == 0;
        const bool noise_enabled = (mixer & static_cast<uint8_t>(mixer_noise_disable_bit << channel)) == 0;
        const bool tone_active = !tone_enabled || tone_channels[channel].high;
        const bool noise_active = !noise_enabled || noise_high;
        const int32_t volume = channel_volume(channel);
        level += (tone_active && noise_active) ? volume : -volume;
    }

    return level;
}

uint16_t AyChip::tone_period(uint8_t channel) const {
    const uint8_t fine = registers[channel * 2];
    const uint8_t coarse = registers[(channel * 2) + 1] & 0x0f;
    const uint16_t period = static_cast<uint16_t>(fine | (coarse << 8));
    return std::max<uint16_t>(period, 1);
}

uint32_t AyChip::tone_half_period(uint8_t channel) const { return static_cast<uint32_t>(tone_period(channel)) * 8; }

uint32_t AyChip::noise_half_period() const {
    const uint8_t period = registers[6] & 0x1f;
    return static_cast<uint32_t>(std::max<uint8_t>(period, 1)) * 8;
}

uint32_t AyChip::envelope_period() const {
    const uint16_t period = static_cast<uint16_t>(registers[11] | (registers[12] << 8));
    return static_cast<uint32_t>(std::max<uint16_t>(period, 1)) * 16;
}

uint8_t AyChip::channel_volume(uint8_t channel) const {
    const uint8_t amplitude = registers[8 + channel];
    if ((amplitude & envelope_enable_bit) == 0) {
        return volume_table[amplitude & 0x0f];
    }

    return volume_table[std::min<uint32_t>(envelope_step, 15)];
}

void AyChip::clock_ay_tick() {
    for (uint8_t channel = 0; channel < tone_channels.size(); ++channel) {
        clock_tone_channel(channel);
    }
    clock_noise();
    clock_envelope();
}

void AyChip::clock_tone_channel(uint8_t channel) {
    ToneChannel &tone = tone_channels[channel];
    tone.counter++;
    if (tone.counter >= tone_half_period(channel)) {
        tone.counter = 0;
        tone.high = !tone.high;
    }
}

void AyChip::clock_noise() {
    noise_counter++;
    if (noise_counter < noise_half_period()) {
        return;
    }
    noise_counter = 0;

    const uint32_t feedback = (noise_lfsr ^ (noise_lfsr >> 3)) & 0x01;
    noise_lfsr = (noise_lfsr >> 1) | (feedback << 16);
    noise_high = (noise_lfsr & 0x01) != 0;
}

void AyChip::clock_envelope() {
    if (envelope_holding) {
        return;
    }

    envelope_counter++;
    if (envelope_counter < envelope_period()) {
        return;
    }
    envelope_counter = 0;

    if (envelope_direction_up && envelope_step < 15) {
        envelope_step++;
        return;
    }
    if (!envelope_direction_up && envelope_step > 0) {
        envelope_step--;
        return;
    }

    const uint8_t shape = registers[13];
    if ((shape & envelope_continue_bit) == 0) {
        envelope_step = 0;
        envelope_holding = true;
        envelope_direction_up = false;
        return;
    }

    if ((shape & envelope_hold_bit) != 0) {
        if ((shape & envelope_alternate_bit) != 0) {
            envelope_direction_up = !envelope_direction_up;
        }
        envelope_holding = true;
        return;
    }

    if ((shape & envelope_alternate_bit) != 0) {
        envelope_direction_up = !envelope_direction_up;
    }
    envelope_step = envelope_direction_up ? 0 : 15;
}

void AyChip::reset_envelope() {
    envelope_counter = 0;
    envelope_holding = false;
    envelope_direction_up = (registers[13] & envelope_attack_bit) != 0;
    envelope_step = envelope_direction_up ? 0 : 15;
}

uint8_t AyChip::sanitized_register_value(uint8_t reg, uint8_t value) {
    switch (reg & 0x0f) {
        case 1:
        case 3:
        case 5:
        case 13:
            return value & 0x0f;
        case 6:
            return value & 0x1f;
        case 8:
        case 9:
        case 10:
            return value & 0x1f;
        default:
            return value;
    }
}
