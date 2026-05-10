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
                                     (machine.border_top * machine.contention_line_tstates)),
          border_timeline(static_cast<std::size_t>(machine.visible_height()) * machine.contention_line_tstates, 0),
          fast_mode(fast_mode) {}
    virtual ~ULA() {}

    void clock(bool &do_exit, bool &do_break);
    uint64_t frame_tstate() const { return counter % machine.frame_tstates; }

private:
    void record_border_tstate(uint64_t frame_pos);
    void render_frame() const;

    MachineModel machine;
    Z80 &_z80;
    Bus &_bus;

    uint64_t counter = {0};
    uint64_t next_frame_deadline = {0};
    uint64_t frame_counter = {0};
    uint32_t visible_frame_start_tstate = {0};
    std::vector<uint8_t> border_timeline;
    bool invert = {false};
    bool fast_mode = {false};
    uint64_t perf_freq = {0};
};
