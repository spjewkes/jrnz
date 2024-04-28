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
        _bus.clock();
        _ula.clock(do_exit, do_break);
        _beeper.clock(false, 0);
        return _z80.clock(_debugger.is_break_enabled()) && !do_exit;
    }
    return false;
}
