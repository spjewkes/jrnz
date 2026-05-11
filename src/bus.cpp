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
        ram_start = machine.ram_base;
    }
}

uint8_t Bus::read_port(uint16_t addr) const {
    account_port_contention(addr);

    // The only port we care about is 0xfe. More specifically for now we just
    // check that the lowest bit is not set. The bits are set as follows: 0-4 :
    // keyboard 5   : unused/high 6   : ear input 7   : unused/high

    if (addr % 2 == 0) {
        uint8_t half_rows = (addr & 0xff00) >> 8;
        uint8_t value = static_cast<uint8_t>(machine.ula_read_high_mask | get_keyboard_state(half_rows));
        if (ear_input_active) {
            value = static_cast<uint8_t>(value | machine.ula_ear_bit_mask);
        }
        return value;
    }

    // Floating bus: when the current beam phase is known, expose the byte the ULA
    // is likely fetching; otherwise keep the older sequential approximation.
    if (frame_tstate_valid) {
        const uint16_t fb_addr = floating_bus_addr_for_tstate(current_frame_tstate);
        if (fb_addr != 0xffff) {
            return mem[fb_addr];
        }
        return 0xff;
    }

    const uint16_t fb_addr = machine.screen_bitmap_base + (floating_counter++ & machine.floating_bus_mask);
    return mem[fb_addr];
}

void Bus::write_port(uint16_t addr, uint8_t v) {
    account_port_contention(addr);

    if ((addr & 0xff) == static_cast<uint8_t>(machine.ula_port & 0xff)) {
        port_254 = v;
    }
}

FetchedOpcode Bus::read_opcode_from_mem(uint16_t addr) const {
    FetchedOpcode fetched{};
    uint16_t curr_addr = addr;

    while (true) {
        const uint8_t opcode = read_data(curr_addr);

        switch (opcode) {
            case 0xdd:
            case 0xfd: {
                const uint8_t next = read_data(curr_addr + 1);

                if (next == 0xdd || next == 0xfd || next == 0xed) {
                    fetched.ignored_prefixes++;
                    curr_addr++;
                    continue;
                }

                fetched.opcode = (static_cast<uint32_t>(opcode) << 8) | next;
                fetched.operand_offset = static_cast<uint16_t>((curr_addr - addr) + 2);
                fetched.fetch_len = static_cast<uint8_t>(fetched.ignored_prefixes + 2);

                if (next == 0xcb) {
                    fetched.opcode = (fetched.opcode << 8) | read_data(curr_addr + 3);
                    // The displacement byte is an operand and does not contribute to the R increment.
                    fetched.fetch_len = static_cast<uint8_t>(fetched.ignored_prefixes + 3);
                }
                return fetched;
            }
            case 0xed:
            case 0xcb:
                fetched.opcode = (static_cast<uint32_t>(opcode) << 8) | read_data(curr_addr + 1);
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
