/**
. * Implement reading of Z80 file format.
 */

#include "bus.hpp"
#include "z80.hpp"

// TODO move this to a utility area
static uint8_t get_next_byte(std::ifstream &stream) {
    char ch;
    stream.get(ch);
    return static_cast<uint8_t>(ch);
}

static uint16_t get_next_ushort(std::ifstream &stream) {
    uint16_t val;
    val = get_next_byte(stream) & 0xff;
    val |= (get_next_byte(stream) << 8) & 0xff00;
    return val;
}

/**
 * @brief Read a data block from a Z80 file into memory.
 */
static void read_data_block(uint32_t version, std::vector<uint8_t> &mem, std::ifstream &stream, bool compressed,
                            uint16_t addr_start, uint16_t size) {
    uint16_t mem_pos = addr_start;

    if (!compressed) {
        // If block of data is uncompressed then just write the remaining file to memory
        stream.read(reinterpret_cast<char *>(&mem[mem_pos]), size);
    } else {
        // while (stream.peek() != EOF || size--) {
        uint32_t pos = 0;
        while (pos < size) {
            if (stream.peek() == EOF) {
                std::cerr << "Warn: Found unexpected end of file at pos " << pos << " of block size " << size << "\n";
                break;
            }

            uint8_t this_byte = get_next_byte(stream);
            pos++;

            if (this_byte == 0xED && stream.peek() == 0xED) {
                // Found a compressed stream of data. Uncompress it.
                uint8_t next_byte = get_next_byte(stream);
                assert(next_byte == 0xED);
                uint8_t count = get_next_byte(stream);
                uint8_t compressed_byte = get_next_byte(stream);
                assert(count > 0);
                while (count--) {
                    mem[mem_pos++] = compressed_byte;
                }
                pos += 3;
            } else if (version == 1 && this_byte == 0x00 && stream.peek() == 0xED) {
                // If dealing with a version 1 file then This could be the end block
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

/**
 * @brief Read the first header.
 */
void read_header_1(std::ifstream &stream, Z80 &state, Bus &bus, uint32_t &version, bool &compression_on) {
    // 0x00 - AF
    state.af.lo(get_next_byte(stream));
    state.af.hi(get_next_byte(stream));

    // 0x02 - BC
    state.bc.lo(get_next_byte(stream));
    state.bc.hi(get_next_byte(stream));

    // 0x04 - HL
    state.hl.lo(get_next_byte(stream));
    state.hl.hi(get_next_byte(stream));

    // 0x06 - PC
    state.pc.lo(get_next_byte(stream));
    state.pc.hi(get_next_byte(stream));
    uint16_t pc = state.pc.get();
    if (pc != 0x0000) {
        version = 1;
    }

    // 0x08 - SP
    state.sp.lo(get_next_byte(stream));
    state.sp.hi(get_next_byte(stream));

    // 0x0A - I
    state.ir.hi(get_next_byte(stream));
    // 0x0B - R - Note that top bit is stored in byte 12
    state.ir.lo(get_next_byte(stream) & 0x7f);

    // 0x0C - Byte 12
    uint8_t byte_12 = get_next_byte(stream);
    // Set top bit of R register
    state.ir.lo(state.ir.lo() | ((byte_12 & 0x1) << 7));
    // Get border color
    uint8_t border_color = (byte_12 >> 1) & 0x7;
    // Get SamRom Basic enabled
    bool samrom_on = static_cast<bool>((byte_12 >> 4) & 0x1);
    UNUSED(samrom_on);
    // Get compression enabled or not
    compression_on = static_cast<bool>((byte_12 >> 5) & 0x1);

    if (byte_12 == 0xff) {
        version = 1;
    }

    // Write border color to port 254
    bus.port_254 &= 0xf8;
    bus.port_254 |= border_color & 0x07;

    // 0x0D - DE
    state.de.lo(get_next_byte(stream));
    state.de.hi(get_next_byte(stream));

    // 0x0F - BC'
    state.bc.swap();
    state.bc.lo(get_next_byte(stream));
    state.bc.hi(get_next_byte(stream));
    state.bc.swap();

    // 0x11 - DE'
    state.de.swap();
    state.de.lo(get_next_byte(stream));
    state.de.hi(get_next_byte(stream));
    state.de.swap();

    // 0x13 - HL'
    state.hl.swap();
    state.hl.lo(get_next_byte(stream));
    state.hl.hi(get_next_byte(stream));
    state.hl.swap();

    // 0x15 - AF'
    state.af.swap();
    state.af.lo(get_next_byte(stream));
    state.af.hi(get_next_byte(stream));
    state.af.swap();

    // 0x17 - IY
    state.iy.lo(get_next_byte(stream));
    state.iy.hi(get_next_byte(stream));

    // 0x19 - IX
    state.ix.lo(get_next_byte(stream));
    state.ix.hi(get_next_byte(stream));

    // 0x1B - Interrupt flipflop, 0=DI, otherwise EI
    bool interrupt_enabled = static_cast<bool>(get_next_byte(stream));
    if (interrupt_enabled) {
        state.iff1 = true;
        state.iff2 = true;
    }

    // 0x1C - IFF2
    uint8_t iff = get_next_byte(stream);
    UNUSED(iff);

    // 0x1D - Byte 29
    uint8_t byte_29 = get_next_byte(stream);
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
}

/**
 * @brief Read the first header.
 */
void read_header_2(std::ifstream &stream, Z80 &state, uint32_t &version) {
    // 0x30 - Length of header 2
    uint16_t length = get_next_ushort(stream);

    if (length == 23) {
        version = 2;
    } else if (length == 54 || length == 55) {
        version = 3;
    } else {
        std::cerr << "Error: unknown version of Z80 file (length " << length << ")\n";
        exit(-1);
    }

    // 0x32 - PC
    state.pc.lo(get_next_byte(stream));
    state.pc.hi(get_next_byte(stream));

    // 0x34 - Hardware mode
    uint8_t hardware_mode = get_next_byte(stream);
    if (hardware_mode != 0) {
        std::cerr << "Error: Only 48k hardware mode is currently supported with Z80 files (mode " << std::hex
                  << static_cast<int>(hardware_mode) << std::dec << ")\n";
        exit(-1);
    }

    // 0x35 - OUT state
    // Ignore this as we only support 48k for now
    uint8_t out_state = get_next_byte(stream);
    UNUSED(out_state);

    // 0x36 - interface 1 ROM paged
    // Ignore this as we neither support interface 1 nor Timex
    uint8_t rom_paged = get_next_byte(stream);
    UNUSED(rom_paged);

    // 0x37 - emulation bits
    // Our emulator ignores this for now
    uint8_t emulation_bits = get_next_byte(stream);
    UNUSED(emulation_bits);

    // 0x38 - last OUT to soundchip
    // Ignore this as we do not support 128k Spectrums
    uint8_t last_out = get_next_byte(stream);
    UNUSED(last_out);

    // 0x39 - contents of the sound chip
    // Our emulator ignores this for now
    char sound_chip_contents[16];
    stream.read(sound_chip_contents, 16);
    UNUSED(sound_chip_contents);

    if (version == 2) {
        return;
    }

    // 0x55 - low T state counter
    // Our emulator ignores this for now
    uint16_t counter_low_t_state = get_next_ushort(stream);
    UNUSED(counter_low_t_state);

    // 0x57 - high T state counter
    // Our emulator ignores this for now
    uint8_t counter_hi_t_state = get_next_byte(stream);
    UNUSED(counter_hi_t_state);

    // 0x58 - flag used by QL emulator
    // This can be ignored
    uint8_t flag_byte_ql = get_next_byte(stream);
    UNUSED(flag_byte_ql);

    // 0x59 - MGT ROM paged
    // Our emulator ignores this for now
    uint8_t mgt_rom_paged = get_next_byte(stream);
    UNUSED(mgt_rom_paged);

    // 0x60 - mulitface ROM paged
    uint8_t multiface_rom_paged = get_next_byte(stream);
    if (multiface_rom_paged != 0) {
        std::cerr << "Warn: Multiface ROM paged not marked as zero in Z80 file\n";
    }

    // 0x61 - 0xff if 0-8191 is ROM, 0 if RAM
    uint8_t bank_0_is_rom = get_next_byte(stream);
    if (bank_0_is_rom != 0xff) {
        std::cerr << "Warn: memory location 0-8191 is not marked as ROM\n";
    }

    // 0x62 - 0xff if 8192-16383 is ROM, 0 if RAM
    uint8_t bank_1_is_rom = get_next_byte(stream);
    if (bank_1_is_rom != 0xff) {
        std::cerr << "Warn: memory location 8192-16383 is not marked as ROM\n";
    }

    // 0x63 - keyboard mappings
    // Our emulator ignores this for now
    char mappings_keyboard[10];
    stream.read(mappings_keyboard, 10);
    UNUSED(mappings_keyboard);

    // 0x73 - ascii words corresponding to keyboard mappings
    // Our emulator ignores this for now
    char mappings_ascii[10];
    stream.read(mappings_ascii, 10);
    UNUSED(mappings_ascii);

    // 0x83 - MGT type
    // Our emulator ignores this for now
    uint8_t mgt_type = get_next_byte(stream);
    UNUSED(mgt_type);

    // 0x84 - Disciple inhibit button
    // Our emulator ignores this for now
    uint8_t disciple_inhibit_button = get_next_byte(stream);
    UNUSED(disciple_inhibit_button);

    // 0x85 - Disciple inhibit flag
    // Our emulator ignores this for now
    uint8_t disciple_inhibit_flag = get_next_byte(stream);
    UNUSED(disciple_inhibit_flag);

    if (length == 55) {
        // 0x86 - last out to port 0x1ffd
        // Our emulator ignores this for now
        uint8_t last_out = get_next_byte(stream);
        UNUSED(last_out);
    }
}

/**
 * @brief Read the first header.
 */
void read_block_header(std::ifstream &stream, uint16_t &length, bool &is_compressed, uint8_t &page) {
    length = get_next_ushort(stream);
    if (length == 0xffff) {
        length = 16384;
        is_compressed = false;
    } else {
        is_compressed = true;
    }
    page = get_next_byte(stream);
}

/**
 * @brief Get start address from page number.
 */
uint16_t get_addr_start_from_page(uint8_t page) {
    switch (page) {
        case 0:
            return 0x0000;
        case 1:
            std::cerr << "Error: interface 1 ROM is not supported\n";
            exit(-1);
        case 2:
            std::cerr << "Error: ROM is 128k mode is not supported\n";
            exit(-1);
        case 3:
            std::cerr << "Error: page 0 in 128k mode is not supported\n";
            exit(-1);
        case 4:
            return 0x8000;
        case 5:
            return 0xc000;
        case 6:
            std::cerr << "Error: page 3 in 128k mode is not supported\n";
            exit(-1);
        case 7:
            std::cerr << "Error: page 4 in 128k mode is not supported\n";
            exit(-1);
        case 8:
            return 0x4000;
        case 9:
            std::cerr << "Error: page 6 in 128k mode is not supported\n";
            exit(-1);
        case 10:
            std::cerr << "Error: page 7 in 128k mode is not supported\n";
            exit(-1);
        case 11:
            std::cerr << "Error: Multiface ROM is not supported\n";
            exit(-1);
        default:
            std::cerr << "Error: unknown page: " << page << std::endl;
            exit(-1);
    }
}

void Bus::load_z80(std::string &z80_file, Z80 &state) {
    if (std::ifstream z80{z80_file, std::ios::binary | std::ios::ate}) {
        uint32_t version = 0;
        auto file_size = z80.tellg();
        UNUSED(file_size);

        z80.seekg(0);

        bool compression_on = false;
        read_header_1(z80, state, *this, version, compression_on);

        if (version != 1) {
            read_header_2(z80, state, version);
            std::cout << "Z80 version " << version << " format detected\n";

            // Read blocks of data into memory
            while (z80.peek() != EOF) {
                uint16_t size = 0;
                bool is_compressed = false;
                uint8_t page = 0;

                read_block_header(z80, size, is_compressed, page);

                uint16_t addr_start = get_addr_start_from_page(page);
                read_data_block(version, mem, z80, is_compressed, addr_start, size);
            }
        } else {
            std::cout << "Z80 version 1 format detected\n";
            // For version one the rest of the block is data
            read_data_block(version, mem, z80, compression_on, 16384, 49152);
        }

        z80.close();
    }
}
