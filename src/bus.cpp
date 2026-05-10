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
        ram_start = MachineConfig48K::ram_base;
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
    uint16_t fb_addr =
        MachineConfig48K::screen_bitmap_base + (floating_counter++ & MachineConfig48K::floating_bus_mask);
    return mem[fb_addr];
}

void Bus::write_port(uint16_t addr, uint8_t v) {
    if ((addr & 0xff) == static_cast<uint8_t>(MachineConfig48K::ula_port & 0xff)) {
        port_254 = v;
    }
}

FetchedOpcode Bus::read_opcode_from_mem(uint16_t addr) const {
    FetchedOpcode fetched{};
    uint16_t curr_addr = addr;

    while (true) {
        const uint8_t opcode = mem[curr_addr];

        switch (opcode) {
            case 0xdd:
            case 0xfd: {
                const uint8_t next = mem[curr_addr + 1];

                if (next == 0xdd || next == 0xfd || next == 0xed) {
                    fetched.ignored_prefixes++;
                    curr_addr++;
                    continue;
                }

                fetched.opcode = (static_cast<uint32_t>(opcode) << 8) | next;
                fetched.operand_offset = static_cast<uint16_t>((curr_addr - addr) + 2);
                fetched.fetch_len = static_cast<uint8_t>(fetched.ignored_prefixes + 2);

                if (next == 0xcb) {
                    fetched.opcode = (fetched.opcode << 8) | mem[curr_addr + 3];
                    // The displacement byte is an operand and does not contribute to the R increment.
                    fetched.fetch_len = static_cast<uint8_t>(fetched.ignored_prefixes + 3);
                }
                return fetched;
            }
            case 0xed:
            case 0xcb:
                fetched.opcode = (static_cast<uint32_t>(opcode) << 8) | mem[curr_addr + 1];
                fetched.operand_offset = static_cast<uint16_t>((curr_addr - addr) + 2);
                fetched.fetch_len = static_cast<uint8_t>((curr_addr - addr) + 2);
                return fetched;
            default:
                fetched.opcode = opcode;
                fetched.operand_offset = static_cast<uint16_t>((curr_addr - addr) + 1);
                fetched.fetch_len = static_cast<uint8_t>((curr_addr - addr) + 1);
                return fetched;
        }
    }
}
