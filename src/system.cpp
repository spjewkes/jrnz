/**
 * @brief Implementation of system class.
 */

#include "system.hpp"

#include <iostream>

bool System::clock() {
    if (do_break) {
        _debugger.set_break(true);
        do_break = false;
    }

    if (_debugger.clock()) {
        uint64_t cycle_count = _z80.total_cycles - last_total_cycles;
        bool is_beeper_on = (_bus.port_254 >> 4) & 0x1;
        bool is_mic_on = !static_cast<bool>(((_bus.port_254) >> 3) & 0x1);
        _beeper.clock(is_beeper_on, is_mic_on, cycle_count);
        last_total_cycles = _z80.total_cycles;

        _bus.clock();
        _ula.clock(do_exit, do_break);
        // _beeper.clock(false, false, 0);
        return _z80.clock(_debugger.is_break_enabled()) && !do_exit;
    }
    return false;
}
