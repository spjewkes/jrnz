/**
 * Implement reading of SNA file format.
 */

#include "z80.hpp"

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
