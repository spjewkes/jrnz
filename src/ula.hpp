/**
 * @brief Header defining the ULA implementation.
 */

#pragma once

#include <cstdint>

#include "bus.hpp"
#include "machine_config.hpp"
#include "z80.hpp"

/**
 * @brief Class describing the ULA.
 */
class ULA {
public:
    ULA(const MachineModel &_machine, Z80 &_z80, Bus &_bus, bool fast_mode = false)
        : machine(_machine), _z80(_z80), _bus(_bus), fast_mode(fast_mode) {}
    virtual ~ULA() {}

    void clock(bool &do_exit, bool &do_break);
    uint64_t frame_tstate() const { return counter % machine.frame_tstates; }

private:
    MachineModel machine;
    Z80 &_z80;
    Bus &_bus;

    uint64_t counter = {0};
    uint64_t next_frame_deadline = {0};
    uint64_t frame_counter = {0};
    bool invert = {false};
    bool fast_mode = {false};
    uint64_t perf_freq = {0};
};
