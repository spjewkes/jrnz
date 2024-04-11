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
    }
}

static uint8_t get_next_byte(std::ifstream &stream) {
    char ch;
    stream.get(ch);
    return static_cast<uint8_t>(ch);
}

void Bus::load_snapshot(std::string &sna_file, Z80 &state) {
    if (std::ifstream sna{sna_file, std::ios::binary | std::ios::ate}) {
        auto file_size = sna.tellg();
        if (file_size != 49179) {
            std::cerr << "WARNING: SNA file size is " << file_size << std::endl;
            std::cerr << "WARNING: Expected 49179 bytes.\n";
        }

        sna.seekg(0);

        // 0x00 - I
        state.ir.hi(get_next_byte(sna));

        // 0x01 - HL'
        state.hl.lo(get_next_byte(sna));
        state.hl.hi(get_next_byte(sna));
        state.hl.swap();

        // 0x03 - DE'
        state.de.lo(get_next_byte(sna));
        state.de.hi(get_next_byte(sna));
        state.de.swap();

        // 0x05 - BC'
        state.bc.lo(get_next_byte(sna));
        state.bc.hi(get_next_byte(sna));
        state.bc.swap();

        // 0x07 - AF'
        state.af.lo(get_next_byte(sna));
        state.af.hi(get_next_byte(sna));
        state.af.swap();

        // 0x09 - HL
        state.hl.lo(get_next_byte(sna));
        state.hl.hi(get_next_byte(sna));

        // 0x0b - DE
        state.de.lo(get_next_byte(sna));
        state.de.hi(get_next_byte(sna));

        // 0x0d - BC
        state.bc.lo(get_next_byte(sna));
        state.bc.hi(get_next_byte(sna));

        // 0x0f - IY
        state.iy.lo(get_next_byte(sna));
        state.iy.hi(get_next_byte(sna));

        // 0x11 - IX
        state.ix.lo(get_next_byte(sna));
        state.ix.hi(get_next_byte(sna));

        // 0x13 - IFF2
        state.iff2 = (get_next_byte(sna) & 0x4 ? true : false);

        // 0x14 - R
        state.ir.lo(get_next_byte(sna));

        // 0x15 - AF
        state.af.lo(get_next_byte(sna));
        state.af.hi(get_next_byte(sna));

        // 0x17 - SP
        state.sp.lo(get_next_byte(sna));
        state.sp.hi(get_next_byte(sna));

        // 0x19 - interrupt mode: 0, 1 or 2
        state.int_mode = get_next_byte(sna);
        assert(state.int_mode == 0 || state.int_mode == 1 || state.int_mode == 2);

        // 0x1a - border colour
        port_254 &= 0xf8;
        port_254 |= get_next_byte(sna) & 0x07;

        //! TODO - ignore for now

        // Renaming file is the 48k of RAM
        sna.read(reinterpret_cast<char *>(&mem[16384]), 49152);
        sna.close();

        // Now execute a RETN instruction
        Instruction inst{InstType::RETN, "retn", 2, 14, Operand::PC};
        state.update_r_reg(inst);
        inst.execute(state);

        std::cout << "Setting PC to: " << state.pc << "\n";

    } else {
        std::cerr << "No SNA file found called " << sna_file << std::endl;
        std::cerr << "SNA failed to load" << std::endl;
    }
}

void Bus::load_z80(std::string &z80_file, Z80 &state) {
    if (std::ifstream z80{z80_file, std::ios::binary | std::ios::ate}) {
        uint32_t version = 0;
        auto file_size = z80.tellg();
        UNUSED(file_size);

        z80.seekg(0);

        // 0x00 - AF
        state.af.lo(get_next_byte(z80));
        state.af.hi(get_next_byte(z80));

        // 0x02 - BC
        state.bc.lo(get_next_byte(z80));
        state.bc.hi(get_next_byte(z80));

        // 0x04 - HL
        state.hl.lo(get_next_byte(z80));
        state.hl.hi(get_next_byte(z80));

        // 0x06 - PC
        state.pc.lo(get_next_byte(z80));
        state.pc.hi(get_next_byte(z80));
        uint16_t pc = state.pc.get();
        if (pc != 0x0000) {
            version = 1;
        }

        // 0x08 - SP
        state.sp.lo(get_next_byte(z80));
        state.sp.hi(get_next_byte(z80));

        // 0x0A - I
        state.ir.hi(get_next_byte(z80));
        // 0x0B - R - Note that top bit is stored in byte 12
        state.ir.lo(get_next_byte(z80) & 0x7f);

        // 0x0C - Byte 12
        uint8_t byte_12 = get_next_byte(z80);
        // Set top bit of R register
        state.ir.lo(state.ir.lo() | ((byte_12 & 0x1) << 7));
        // Get border color
        uint8_t border_color = (byte_12 >> 1) & 0x7;
        // Get SamRom Basic enabled
        bool samrom_on = static_cast<bool>((byte_12 >> 4) & 0x1);
        UNUSED(samrom_on);
        // Get compression enabled or not
        bool compression_on = static_cast<bool>((byte_12 >> 5) & 0x1);

        // Write border color to port 254
        port_254 &= 0xf8;
        port_254 |= border_color & 0x07;

        // 0x0D - DE
        state.de.lo(get_next_byte(z80));
        state.de.hi(get_next_byte(z80));

        // 0x0F - BC'
        state.bc.swap();
        state.bc.lo(get_next_byte(z80));
        state.bc.hi(get_next_byte(z80));
        state.bc.swap();

        // 0x11 - DE'
        state.de.swap();
        state.de.lo(get_next_byte(z80));
        state.de.hi(get_next_byte(z80));
        state.de.swap();

        // 0x13 - HL'
        state.hl.swap();
        state.hl.lo(get_next_byte(z80));
        state.hl.hi(get_next_byte(z80));
        state.hl.swap();

        // 0x15 - AF'
        state.af.swap();
        state.af.lo(get_next_byte(z80));
        state.af.hi(get_next_byte(z80));
        state.af.swap();

        // 0x17 - IY
        state.iy.lo(get_next_byte(z80));
        state.iy.hi(get_next_byte(z80));

        // 0x19 - IX
        state.ix.lo(get_next_byte(z80));
        state.ix.hi(get_next_byte(z80));

        // 0x1B - Interrupt flipflop, 0=DI, otherwise EI
        bool interrupt_enabled = static_cast<bool>(get_next_byte(z80));
        if (interrupt_enabled) {
            state.iff1 = true;
            state.iff2 = true;
        }

        // 0x1C - IFF2
        uint8_t iff = get_next_byte(z80);
        UNUSED(iff);

        // 0x1D - Byte 29
        uint8_t byte_29 = get_next_byte(z80);
        // Interrupt mode 0, 1, or 2
        state.int_mode = byte_29 & 0x3;
        // Issue 2 enabled
        bool issue2_enabled = static_cast<bool>((byte_29 >> 2) & 0x1);
        UNUSED(issue2_enabled);
        // Double interrupt frequency
        bool double_interrupt_freq = static_cast<bool>((byte_29 >> 3) & 0x1);
        UNUSED(double_interrupt_freq);
        // Video synchronization
        uint8_t video_sync = (byte_29 >> 4) & 0x3;
        UNUSED(video_sync);
        // Joystick selected
        uint8_t joystick_selected = (byte_29 >> 6) & 0x3;
        UNUSED(joystick_selected);

        if (version != 1) {
            std::cerr << "Only Z80 version 1 files are currently supported\n";
            exit(-1);
        }

        if (!compression_on) {
            // If block of data is uncompressed then just write the remaining file to memory
            z80.read(reinterpret_cast<char *>(&mem[16384]), 49152);
        } else {
            uint8_t this_byte = get_next_byte(z80);
            uint16_t mem_pos = 16384;

            while (!z80.eof()) {
                uint8_t next_byte = get_next_byte(z80);
                if (z80.eof()) {
                    // Write byte to memory before exiting loop
                    mem[mem_pos++] = this_byte;
                    break;
                } else if (this_byte == 0xED && next_byte == 0xED) {
                    uint8_t count = get_next_byte(z80);
                    uint8_t compressed_byte = get_next_byte(z80);
                    while (count--) {
                        mem[mem_pos++] = compressed_byte;
                    }
                } else {
                    // Normal byte - write it to memory
                    mem[mem_pos++] = this_byte;
                }

                // Shuffle bytes
                this_byte = next_byte;
            }
        }

        z80.close();
    }
}

uint8_t Bus::read_port(uint16_t addr) const {
    // The only port we care about is 0xfe. More specifically for now we just
    // check that the lowest bit is not set. The bits are set as follows: 0-4 :
    // keyboard 5   : unused 6   : ear 7   : unused
    //! ear is currently not handled

    if (addr % 2 == 0) {
        uint8_t half_rows = (addr & 0xff00) >> 8;
        return get_keyboard_state(half_rows);
    }

    return 0;
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
