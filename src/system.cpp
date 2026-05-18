/**
 * @brief Implementation of system class.
 */

#include "system.hpp"

#include <iostream>

void System::sync_ay_registers() {
    for (uint8_t reg = 0; reg < 16; ++reg) {
        _ay.write_register(reg, _bus.ay_register(reg));
    }
}

bool System::clock() {
    if (do_break) {
        _debugger.set_break(true);
        do_break = false;
    }

    if (_debugger.clock()) {
        uint64_t cycle_count = 1;
        bool is_beeper_on = (_bus.port_254 >> 4) & 0x1;
        bool is_mic_on = !static_cast<bool>(((_bus.port_254) >> 3) & 0x1);
        sync_ay_registers();
        _ay.clock_cpu_tstate();
        _beeper.clock(is_beeper_on, is_mic_on, _ay.output_level(), cycle_count);

        _bus.clock();
        // The CPU samples contention against the ULA phase for the current tick,
        // so publish the current frame t-state before advancing the ULA.
        _bus.set_frame_tstate(_ula.frame_tstate());
        _ula.clock(do_exit, do_break);
        if (_tape != nullptr && _tape->try_fast_load(_z80)) {
            return !do_exit;
        }
        // _beeper.clock(false, false, 0);
        return _z80.clock(_debugger.is_break_enabled()) && !do_exit;
    }
    return false;
}
