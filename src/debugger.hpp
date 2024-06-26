/**
 * @brief Class that handles the debugger.
 */

#pragma once

#include <cstdlib>

#include "bus.hpp"
#include "z80.hpp"

/**
 * Defines the debugger class.
 */
class Debugger {
public:
    Debugger(Z80 &_z80, Bus &_bus) : _z80(_z80), _bus(_bus) {}
    virtual ~Debugger() {}

    void set_dout(bool enable) { debug_out = enable; }
    void set_break(bool enable);
    void set_break(bool enable, uint16_t _break_pc);
    bool break_ready();
    bool is_break_enabled() const { return break_enabled; }

    bool clock();

    std::stringstream dump_instr_at_addr(uint16_t addr);
    std::stringstream dump_mem_at_addr(uint16_t addr, size_t size, size_t per_line = 16);

    void dump();
    void dump_sp();

public:
    Z80 &_z80;
    Bus &_bus;

    bool debug_out = {false};
    size_t break_step = {0ull};

    // TODO: really need to tidy this up
    bool break_enabled = {false};
    bool break_at_pc = {false};
    bool break_at_breakpoint = {false};
    uint16_t break_pc = {0x0000};
    uint16_t break_pc_tmp = {0x0000};
    uint16_t break_breakpoint = {0x0000};
};
