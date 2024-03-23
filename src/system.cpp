/**
 * @brief Implementation of system class.
 */

#include "system.hpp"

#include <iostream>

bool System::clock() {
    if (do_break) {
        _debugger.set_break(true);
    }

    if (_debugger.clock()) {
        _bus.clock();
        _ula.clock(do_exit, do_break);
        return _z80.clock(_debugger.is_break_enabled()) && !do_exit;
    }
    return false;
}
