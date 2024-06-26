/**
 * @brief Header defining the core of the z80 implementation.
 */

#pragma once

#include <cstdint>

#include "bus.hpp"
#include "instructions.hpp"
#include "register.hpp"

/**
 * @brief Class describing a Z80 state.
 */
class Z80 {
public:
    Z80(Bus &_bus, bool fast_mode = false);

    uint16_t curr_opcode_pc = {0};   // Stores the PC of the opcode under execution
    uint16_t curr_operand_pc = {0};  // Stores the PC of the expected first operand (if there are any) of
                                     // the opcode under execution
    uint16_t top_of_stack = {0};     // Stores the expected top of the stack (for aiding debugging)
    Register16 pc;
    Register16 sp;
    Register16 ix;
    Register16 iy;

    RegisterAF af;
    Register16 hl;
    Register16 bc;
    Register16 de;

    bool iff1 = {false};
    bool iff2 = {false};

    uint8_t int_mode = {0};
    bool int_nmi = {false};
    bool interrupt = {false};
    bool halted = {false};

    uint32_t cycles_left = {0};
    uint64_t total_cycles = {0};

    Bus &bus;

    Register16 ir;

    bool clock(bool no_cycles = false);

    void reset();

    void update_r_reg(const Instruction &inst, uint32_t opcode = 0x00);

private:
    bool fast_mode = {false};
};
