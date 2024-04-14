/**
 * @brief Implementation of z80 state class.
 */

#include "z80.hpp"

#include "decoder.hpp"

Z80::Z80(Bus &_bus, bool fast_mode) : bus(_bus), fast_mode(fast_mode) { reset(); }

bool Z80::clock(bool no_cycles) {
    bool found = false;

    if (fast_mode || no_cycles) {
        // Ignore instruction cycle count. This is typically used by the debugger to
        // allow it to single step through the code
        cycles_left = 0;
    }

    // Cycles left means that repeated clocking of the Z80 waits the number of cycles on each instruction
    // executed. This gives the emulation roughly the right behaviour for each instruction.
    if (cycles_left == 0) {
        if (int_nmi) {
            Instruction inst{InstType::PUSH, "NMI", 1, 11, Operand::UNUSED, Operand::PC};
            update_r_reg(inst);
            cycles_left = inst.execute(*this);
            pc.set(0x66);
            iff2 = iff1;
            iff1 = false;
            int_nmi = false;
            halted = false;
            found = true;
        } else if (iff1 && interrupt) {
            halted = false;
            switch (int_mode) {
                case 0:
                /* TODO mode 0 should be made more generic than this but for the ZX Spectrum we can
                   just make it emulate the mode 1 interrupt */
                case 1: {
                    Instruction inst{InstType::PUSH, "INT1", 1, 13, Operand::UNUSED, Operand::PC};
                    update_r_reg(inst);
                    cycles_left = inst.execute(*this);
                    pc.set(0x38);
                    interrupt = false;
                    found = true;
                    break;
                }
                case 2: {
                    Instruction inst{InstType::PUSH, "INT2", 1, 13, Operand::UNUSED, Operand::PC};
                    cycles_left = inst.execute(*this);
                    uint16_t read_addr = ir.get() & 0xff00;
                    uint16_t jump_addr = bus.read_addr_from_mem(read_addr);
                    pc.set(jump_addr);
                    interrupt = false;
                    found = true;
                    break;
                }
            }
        } else if (halted) {
            // The halt instruction will continuously execute NOPs until there is an
            // interrupt
            Instruction inst{InstType::NOP, "halt", 1, 4};
            update_r_reg(inst);
            cycles_left = const_cast<Instruction &>(inst).execute(*this);
            found = true;
        } else {
            curr_opcode_pc = pc.get();

            uint16_t operand_offset = 0;
            const auto opcode = bus.read_opcode_from_mem(curr_opcode_pc, &operand_offset);
            assert(operand_offset != 0);
            curr_operand_pc = curr_opcode_pc + operand_offset;

            const Instruction &inst = decode_opcode(opcode);
            update_r_reg(inst, opcode);
            if (inst.inst != InstType::INV) {
                pc.set(curr_opcode_pc + inst.size);
                cycles_left = const_cast<Instruction &>(inst).execute(*this);
                found = true;
            } else {
                std::cerr << "UNKNOWN OPCODE: 0x" << std::hex << std::setw(8) << std::setfill('0') << opcode;
                std::cerr << " at 0x" << curr_opcode_pc << std::endl;
            }
        }
    } else {
        cycles_left--;
        found = true;
    }

    return found;
}

void Z80::reset() {
    pc.reset();
    af.reset();
    sp.reset();

    af.set(0xffff);
    sp.set(0xffff);

    top_of_stack = 0xffff;

    iff1 = false;
    iff2 = false;
    int_mode = 0;
}

void Z80::update_r_reg(const Instruction &inst, uint32_t opcode) {
    uint8_t r = ir.lo();

    if (((opcode & 0xcb00) == 0xcb00) || ((opcode & 0xed00) == 0xed00) ||
        (inst.inst == InstType::LD && inst.dst == Operand::R && inst.src == Operand::A) ||
        (inst.inst == InstType::LD && inst.dst == Operand::A && inst.src == Operand::R) ||
        (inst.inst == InstType::LDDR) || (inst.inst == InstType::LDIR)) {
        r++;
    }

    r++;
    r &= 0x7f;
    ir.lo(r);
}
