/**
 * @brief Centralized machine constants for the ZX Spectrum 48K model.
 */

#pragma once

#include <cstddef>
#include <cstdint>

struct MachineConfig48K {
    static constexpr std::size_t memory_size = 65536;

    static constexpr uint16_t rom_base = 0x0000;
    static constexpr uint16_t ram_base = 0x4000;
    static constexpr uint16_t screen_bitmap_base = 0x4000;
    static constexpr uint16_t screen_attr_base = 0x5800;
    static constexpr uint16_t floating_bus_mask = 0x3fff;

    static constexpr uint16_t ula_port = 0x00fe;
    static constexpr uint8_t interrupt_hold_tstates = 32;

    static constexpr uint32_t cpu_frequency_hz = 3500000;
    static constexpr uint32_t frame_rate_hz = 50;
    static constexpr uint32_t frame_tstates = 69888;

    static constexpr int screen_width = 256;
    static constexpr int screen_height = 192;
    static constexpr int attr_cell_size = 8;
    static constexpr int border_left = 32;
    static constexpr int border_top = 32;
    static constexpr int visible_width = screen_width + (border_left * 2);
    static constexpr int visible_height = screen_height + (border_top * 2);

    static constexpr float render_scale = 3.0f;
    static constexpr int window_width = static_cast<int>(visible_width * render_scale);
    static constexpr int window_height = static_cast<int>(visible_height * render_scale);
};
