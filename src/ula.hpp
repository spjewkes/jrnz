/**
 * @brief Header defining the ULA implementation.
 */

#ifndef __ULA_HPP__
#define __ULA_HPP__

#include <cstdint>

#include "bus.hpp"
#include "z80.hpp"

/**
 * @brief Class describing the ULA.
 */
class ULA {
public:
    ULA(Z80 &_z80, Bus &_bus, bool fast_mode = false) : _z80(_z80), _bus(_bus), fast_mode(fast_mode) {}
    virtual ~ULA() {}

    void clock(bool &do_exit, bool &do_break);

private:
    Z80 &_z80;
    Bus &_bus;

    uint64_t counter = {0};
    uint32_t next_frame_ticks = {0};
    uint64_t frame_counter = {0};
    bool invert = {false};
    bool fast_mode = {false};
};

#endif  // __ULA_HPP__
