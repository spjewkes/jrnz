/**
 * Implementation of the memory/data bus system.
 */

#include "bus.hpp"

#include "z80.hpp"

void Bus::load_rom(std::string &rom_file) {
    if (std::ifstream rom{rom_file, std::ios::binary | std::ios::ate}) {
        auto rom_size = rom.tellg();
        ram_start = rom_size;
        rom.seekg(0);
        rom.read(reinterpret_cast<char *>(&mem[0]), rom_size);
        rom.close();
    } else {
        std::cerr << "No ROM file found called " << rom_file << std::endl;
        std::cerr << "ROM uninitialized" << std::endl;
        ram_start = 0x4000;
    }
}

uint8_t Bus::read_port(uint16_t addr) const {
    // The only port we care about is 0xfe. More specifically for now we just
    // check that the lowest bit is not set. The bits are set as follows: 0-4 :
    // keyboard 5   : unused 6   : ear 7   : unused
    //! ear is currently not handled

    if (addr % 2 == 0) {
        uint8_t half_rows = (addr & 0xff00) >> 8;
        return static_cast<uint8_t>(0xe0 | get_keyboard_state(half_rows));
    }

    // Floating bus: return a byte from screen/attribute memory that changes over time.
    // Note: keep counter outside const interface by not incrementing here.
    uint16_t fb_addr = 0x4000 + (floating_counter & 0x3fff);
    return mem[fb_addr];
}

void Bus::write_port(uint16_t addr, uint8_t v) {
    if ((addr & 0xff) == 0xfe) {
        port_254 = v;
    }
}

uint32_t Bus::read_opcode_from_mem(uint16_t addr, uint16_t *operand_offset) {
    uint16_t offset = 1;
    uint32_t opcode = mem[addr];

    // Handled extended instructions
    switch (opcode) {
        case 0xed:
        case 0xcb:
        case 0xdd:
        case 0xfd: {
            opcode = (opcode << 8) | mem[addr + 1];
            offset++;

            // Handle IX and IY bit instructions, the opcode comes after
            // the displacement byte:
            // 0xddcb <displacement byte> <opcode>
            // oxfdcb <displacement byte> <opcode>
            // Make sure the operand is not in the returned opcode
            switch (opcode) {
                case 0xddcb:
                case 0xfdcb:
                    opcode = (opcode << 8) | mem[addr + 3];
            }
        }
    }

    if (operand_offset != nullptr) {
        *operand_offset = offset;
    }

    return opcode;
}
