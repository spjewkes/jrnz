/**
 * Implement reading of Z80 file format.
 */

#include "z80.hpp"

// TODO move this to a utility area
static uint8_t get_next_byte(std::ifstream &stream) {
    char ch;
    stream.get(ch);
    return static_cast<uint8_t>(ch);
}

static void read_data_block(std::vector<uint8_t> &mem, std::ifstream &stream, bool compressed, uint16_t addr_start,
                            uint16_t size) {
    uint16_t mem_pos = addr_start;

    if (!compressed) {
        // If block of data is uncompressed then just write the remaining file to memory
        stream.read(reinterpret_cast<char *>(&mem[mem_pos]), size);
    } else {
        while (stream.peek() != EOF) {
            uint8_t this_byte = get_next_byte(stream);

            if (this_byte == 0xED && stream.peek() == 0xED) {
                // Found a compressed stream of data. Uncompress it.
                uint8_t next_byte = get_next_byte(stream);
                assert(next_byte == 0xED);
                uint8_t count = get_next_byte(stream);
                uint8_t compressed_byte = get_next_byte(stream);
                while (count--) {
                    mem[mem_pos++] = compressed_byte;
                }
            } else if (this_byte == 0x00 && stream.peek() == 0xED) {
                // This could be the end block
                uint8_t byte_2 = get_next_byte(stream);
                uint8_t byte_3 = get_next_byte(stream);
                uint8_t byte_4 = get_next_byte(stream);
                if (byte_3 == 0xED && byte_4 == 0x00) {
                    // Block end reached
                    std::cout << "Z80 Block end found at: " << static_cast<uint16_t>(mem_pos - 1) << std::endl;
                    break;
                } else {
                    // Not the end of memory, so write first byte out and put back
                    // the remaining 3 bytes
                    mem[mem_pos++] = this_byte;
                    stream.putback(byte_4);
                    stream.putback(byte_3);
                    stream.putback(byte_2);
                }
            } else {
                // Normal byte - write it to memory
                mem[mem_pos++] = this_byte;
            }
        }
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
        } else {
            // For version one the rest of the block is data
            read_data_block(mem, z80, compression_on, 16384, 49152);
        }

        z80.close();
    }
}
