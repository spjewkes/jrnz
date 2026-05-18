/**
 * @brief Class that manage entire system.
 */

#pragma once

#include <cstdlib>

#include "beeper.hpp"
#include "bus.hpp"
#include "debugger.hpp"
#include "tape.hpp"
#include "ula.hpp"
#include "z80.hpp"

/**
 * Defines the system class.
 */
class System {
public:
    System(Z80 &_z80, ULA &_ula, Bus &_bus, Debugger &_debugger, Beeper &_beeper, TapeDeck *_tape = nullptr)
        : _z80(_z80), _ula(_ula), _bus(_bus), _debugger(_debugger), _beeper(_beeper), _tape(_tape) {}
    virtual ~System() {}

    bool clock();

    Z80 &z80() { return _z80; }
    ULA &ula() { return _ula; }
    Bus &bus() { return _bus; }
    Debugger &debugger() { return _debugger; }

private:
    Z80 &_z80;
    ULA &_ula;
    Bus &_bus;
    Debugger &_debugger;
    Beeper &_beeper;
    TapeDeck *_tape;

    bool do_exit = {false};
    bool do_break = {false};
};
