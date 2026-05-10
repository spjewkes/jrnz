/**
 * @brief Machine model definitions and default ZX Spectrum 48K configuration.
 */

#pragma once

#include <cstddef>
#include <cstdint>

struct MachineModel {
    std::size_t memory_size;

    uint16_t rom_base;
    uint16_t ram_base;
    uint16_t screen_bitmap_base;
    uint16_t screen_attr_base;
    uint16_t floating_bus_mask;

    uint16_t ula_port;
    uint8_t interrupt_hold_tstates;

    uint32_t cpu_frequency_hz;
    uint32_t frame_rate_hz;
    uint32_t frame_tstates;

    int screen_width;
    int screen_height;
    int attr_cell_size;
    int border_left;
    int border_top;

    float render_scale;

    constexpr int visible_width() const { return screen_width + (border_left * 2); }
    constexpr int visible_height() const { return screen_height + (border_top * 2); }
    constexpr int window_width() const { return static_cast<int>(visible_width() * render_scale); }
    constexpr int window_height() const { return static_cast<int>(visible_height() * render_scale); }
};

constexpr MachineModel spectrum_48k_model() {
    return MachineModel{
        .memory_size = 65536,
        .rom_base = 0x0000,
        .ram_base = 0x4000,
        .screen_bitmap_base = 0x4000,
        .screen_attr_base = 0x5800,
        .floating_bus_mask = 0x3fff,
        .ula_port = 0x00fe,
        .interrupt_hold_tstates = 32,
        .cpu_frequency_hz = 3500000,
        .frame_rate_hz = 50,
        .frame_tstates = 69888,
        .screen_width = 256,
        .screen_height = 192,
        .attr_cell_size = 8,
        .border_left = 32,
        .border_top = 32,
        .render_scale = 3.0f,
    };
}
