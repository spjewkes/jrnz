/**
 * @brief Implementation of debugger class.
 */

#include "debugger.hpp"

#include "decoder.hpp"

void Debugger::set_break(bool enable) { set_break(enable, _z80.pc.get()); }

void Debugger::set_break(bool enable, uint16_t _break_pc) {
    break_at_pc = enable;
    break_pc = _break_pc;
}

bool Debugger::break_ready() {
    if (break_at_pc && (_z80.pc.get() == break_pc || (_z80.pc.get() == break_pc_tmp && 0x0000 != break_pc_tmp))) {
        std::cout << "Enabled break at 0x" << std::hex << break_pc << std::dec << std::endl;
        break_enabled = true;
        break_pc_tmp = 0x0000;
        return true;
    }

    if (break_at_breakpoint && _z80.pc.get() == break_breakpoint) {
        std::cout << "Enabled break at 0x" << std::hex << break_breakpoint << std::dec << std::endl;
        break_enabled = true;
        return true;
    }

    return false;
}

bool Debugger::clock() {
    bool running = true;

    if ((break_enabled && !break_step) || break_ready()) {
        bool paused = true;
        char ch;

        std::cout << "DEBUG - break enabled\n";

        while (paused) {
            std::cout << "Next instruction: " << dump_instr_at_addr(_z80.pc.get()).str() << std::endl;
            std::cin >> ch;

            switch (ch) {
                case 'b':
                    break_at_breakpoint = true;
                    std::cin >> std::hex >> break_breakpoint;
                    break;
                case 'x':
                    break_at_breakpoint = false;
                    break;
                case 'c':
                    paused = false;
                    break_enabled = false;
                    break_at_pc = false;
                    break;
                case 's':
                    std::cin >> break_step;
                    paused = false;
                    break;
                case 'r':
                    dump();
                    break;
                case 't':
                    dump_sp();
                    break;
                case 'd': {
                    std::string str_offset;
                    std::string str_size;
                    std::cin >> str_offset >> str_size;
                    size_t offset = strtoul(str_offset.c_str(), NULL, 0);
                    size_t size = strtoul(str_size.c_str(), NULL, 0);
                    std::cout << dump_mem_at_addr(offset, size).str() << std::endl;
                    break;
                }
                case 'n':
                    paused = false;
                    break;
                case 'u': {
                    StorageElement stack_pc = StorageElement::create_element(_z80, Operand::indSP);
                    uint32_t val;
                    stack_pc.get_value(val);
                    break_pc_tmp = val;
                    break_at_pc = true;
                    paused = false;
                    break_enabled = false;
                    break;
                }
                case 'i':
                    _z80.int_nmi = true;
                    break;
                case 'q':
                    running = false;
                    paused = false;
                    break;
                default:
                    std::string help_text =
                        "In debug mode.\n"
                        "Help:\n"
                        "\tb <addr> = set breakpoint at <addr>\n"
                        "\tx = clear breakpoint\n"
                        "\tc = continue\n"
                        "\ts <n> = step <n> times\n"
                        "\tr = dump registers\n"
                        "\tt = dump stack\n"
                        "\td <offset> <size> = dump memory contents starting at <offset> "
                        "for <size> bytes\n"
                        "\tn = next instruction\n"
                        "\tu = continue to address on sp (tries to jump out of a "
                        "routine)\n"
                        "\ti = NMI\n"
                        "\tq = quit\n";
                    std::cout << help_text;
                    break;
            }
        }
    } else {
        // Only dump current instruction we're not clocking the current instruction
        if (debug_out && !_z80.cycles_left) {
            std::cout << dump_instr_at_addr(_z80.pc.get()).str() << std::endl;
        }
    }

    if (break_step) {
        break_step--;
    }

    return running;
}

std::stringstream Debugger::dump_instr_at_addr(uint16_t addr) {
    std::stringstream str;

    const auto opcode = _bus.read_opcode_from_mem(addr);
    const Instruction &inst = decode_opcode(opcode);
    if (inst.inst != InstType::INV) {
        str << std::left << std::setw(20) << dump_mem_at_addr(addr, inst.size).str();
        str << std::setw(20) << inst.name;

        if (has_rom_label(addr)) {
            str << "Routine: " << decode_rom_label(addr);
        }
    } else {
        str << dump_mem_at_addr(addr, 4).str() << " UNKNOWN INSTRUCTION: 0x" << std::hex << opcode << std::dec;
    }

    return str;
}

std::stringstream Debugger::dump_mem_at_addr(uint16_t addr, size_t size, size_t per_line) {
    std::stringstream str;

    uint16_t curr_addr = addr;

    for (size_t pos = 0; pos < size; pos++) {
        if ((pos % per_line) == 0) {
            str << std::hex << "0x" << std::setw(4) << std::setfill('0') << curr_addr << ":";
        }

        str << std::hex << " " << std::setw(2) << std::setfill('0')
            << static_cast<unsigned int>(_bus.read_data(curr_addr));

        if ((pos % per_line) == (per_line - 1)) {
            str << std::endl;
        }

        curr_addr++;
    }

    return str;
}

void Debugger::dump() {
    std::cout << "PC: " << _z80.pc << "   \t";
    std::cout << "SP: " << _z80.sp << std::endl;

    std::cout << "AF: " << _z80.af << "   \t";
    _z80.af.swap();
    std::cout << "AF\': " << _z80.af << std::endl;
    _z80.af.swap();

    std::cout << "BC: " << _z80.bc << "   \t";
    _z80.bc.swap();
    std::cout << "BC\': " << _z80.bc << std::endl;
    _z80.bc.swap();

    std::cout << "DE: " << _z80.de << "   \t";
    _z80.de.swap();
    std::cout << "DE\': " << _z80.de << std::endl;
    _z80.de.swap();

    std::cout << "HL: " << _z80.hl << "   \t";
    _z80.hl.swap();
    std::cout << "HL\': " << _z80.hl << std::endl;
    _z80.hl.swap();

    std::cout << "IX: " << _z80.ix << "   \t";
    std::cout << "IY: " << _z80.iy << std::endl;

    std::cout << "I: 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(_z80.ir.hi())
              << " - ";
    std::cout << "R: 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(_z80.ir.lo())
              << std::endl;

    std::cout << "IM: " << static_cast<uint32_t>(_z80.int_mode);
    std::cout << " IFF1: " << (_z80.iff1 ? "on" : "off");
    std::cout << " IFF2: " << (_z80.iff2 ? "on" : "off");
    std::cout << std::endl;

    std::cout << "Flags: " << _z80.af.dump_flags().str() << std::endl;
}
void Debugger::dump_sp() {
    assert(_z80.sp.get() <= _z80.top_of_stack);
    std::cout << "Dumping stack at SP: " << _z80.sp << std::endl;
    std::cout << dump_mem_at_addr(_z80.sp.get(), _z80.top_of_stack - _z80.sp.get()).str() << std::endl;
    std::cout << "==== TOP OF THE STACK ====" << std::endl;
}
