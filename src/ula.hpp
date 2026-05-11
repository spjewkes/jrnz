/**
 * @brief Header defining the ULA implementation.
 */

#pragma once

#include <cstdint>
#include <vector>

#include "bus.hpp"
#include "machine_config.hpp"
#include "z80.hpp"

/**
 * @brief Class describing the ULA.
 */
class ULA {
public:
    ULA(const MachineModel &_machine, Z80 &_z80, Bus &_bus, bool fast_mode = false)
        : machine(_machine),
          _z80(_z80),
          _bus(_bus),
          visible_frame_start_tstate(machine.contention_first_tstate -
                                     (machine.vertical_blank_top_lines * machine.contention_line_tstates) -
                                     (machine.horizontal_border_left_tstates + machine.horizontal_blank_left_tstates)),
          border_timeline(static_cast<std::size_t>(machine.visible_height()) * horizontal_visible_tstates(), 0),
          screen_bitmap_snapshot(static_cast<std::size_t>(machine.screen_width / machine.attr_cell_size) *
                                     static_cast<std::size_t>(machine.screen_height),
                                 0),
          screen_attr_snapshot(static_cast<std::size_t>(machine.screen_width / machine.attr_cell_size) *
                                   static_cast<std::size_t>(machine.screen_height),
                               0),
          fast_mode(fast_mode) {}
    virtual ~ULA() {}

    void clock(bool &do_exit, bool &do_break);
    uint64_t frame_tstate() const { return counter % machine.frame_tstates; }

private:
    constexpr std::size_t horizontal_visible_tstates() const {
        return static_cast<std::size_t>(machine.horizontal_border_left_tstates) + machine.contention_visible_tstates +
               machine.horizontal_border_right_tstates;
    }
    void record_border_tstate(uint64_t frame_pos);
    void record_screen_tstate(uint64_t frame_pos);
    void render_frame() const;
    static uint8_t remap_spectrum_y(uint8_t y);

    MachineModel machine;
    Z80 &_z80;
    Bus &_bus;

    uint64_t counter = {0};
    uint64_t next_frame_deadline = {0};
    uint64_t frame_counter = {0};
    uint32_t visible_frame_start_tstate = {0};
    std::vector<uint8_t> border_timeline;
    std::vector<uint8_t> screen_bitmap_snapshot;
    std::vector<uint8_t> screen_attr_snapshot;
    bool invert = {false};
    bool fast_mode = {false};
    uint64_t perf_freq = {0};
};
