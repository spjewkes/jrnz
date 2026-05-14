/**
 * Implement reading of SNA file format.
 */

#include "z80.hpp"

// TODO move this to a utility area
static uint8_t get_next_byte(std::ifstream &stream) {
    char ch;
    stream.get(ch);
    return static_cast<uint8_t>(ch);
}

void Bus::load_snapshot(std::string &sna_file, Z80 &state) {
    if (std::ifstream sna{sna_file, std::ios::binary | std::ios::ate}) {
        state.reset();
        port_254 = 0;
        floating_counter = 0;

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

        // The rest of a 48K SNA file is the 48K RAM image at 0x4000-0xffff.
        for (uint32_t offset = 0; offset < 49152; ++offset) {
            (*this)[static_cast<uint16_t>(0x4000 + offset)] = get_next_byte(sna);
        }
        sna.close();

        // 48K SNA stores the return PC on the stack; loading should restore it
        // directly without mutating the saved R value.
        state.pc.set(read_addr_from_mem(state.sp.get()));
        state.sp.set(static_cast<uint16_t>(state.sp.get() + 2));
        state.iff1 = state.iff2;

        std::cout << "Setting PC to: " << state.pc << "\n";

    } else {
        std::cerr << "No SNA file found called \'" << sna_file << "\'" << std::endl;
        std::cerr << "SNA file failed to load" << std::endl;
    }
}
