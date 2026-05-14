#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <string>

#include "bus.hpp"
#include "z80.hpp"

namespace {
class TempFile {
public:
    explicit TempFile(const char *name) : path(std::string("/tmp/") + name) {}
    ~TempFile() { std::remove(path.c_str()); }

    std::string path;
};
}  // namespace

TEST_CASE("Z80 snapshot loader restores AF and interrupt flip-flops from the file header", "[snapshot][z80]") {
    TempFile tmp("jrnz_test_snapshot.z80");
    std::ofstream out(tmp.path, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());

    const std::array<uint8_t, 30> header = {
        0x12, 0x34,  // A, F
        0x78, 0x56,  // C, B
        0xbc, 0x9a,  // L, H
        0x00, 0x80,  // PC
        0xfe, 0xff,  // SP
        0x44,        // I
        0x55,        // R low 7 bits
        0x02,        // border color in bits 1-3, top R bit clear, uncompressed
        0xf0, 0xde,  // E, D
        0x21, 0x43,  // C', B'
        0x65, 0x87,  // E', D'
        0xa9, 0xcb,  // L', H'
        0xee, 0xdd,  // A', F'
        0x34, 0x12,  // IY
        0x78, 0x56,  // IX
        0x00,        // IFF1
        0x01,        // IFF2
        0x02         // IM 2
    };
    out.write(reinterpret_cast<const char *>(header.data()), static_cast<std::streamsize>(header.size()));

    std::array<uint8_t, 49152> ram{};
    ram[0] = 0xaa;
    ram[1] = 0xbb;
    ram[2] = 0xcc;
    out.write(reinterpret_cast<const char *>(ram.data()), static_cast<std::streamsize>(ram.size()));
    out.close();

    Bus bus(65536);
    Z80 state(bus, true);
    state.af.set(0xffff);
    state.af.swap();
    state.af.set(0x0000);
    state.af.swap();
    state.iff1 = true;
    state.iff2 = false;

    bus.load_z80(tmp.path, state);

    REQUIRE(state.af.accum() == 0x12);
    REQUIRE(state.af.flags() == 0x34);
    state.af.swap();
    REQUIRE(state.af.accum() == 0xee);
    REQUIRE(state.af.flags() == 0xdd);
    state.af.swap();

    REQUIRE(state.bc.get() == 0x5678);
    REQUIRE(state.hl.get() == 0x9abc);
    REQUIRE(state.de.get() == 0xdef0);
    REQUIRE(state.ix.get() == 0x5678);
    REQUIRE(state.iy.get() == 0x1234);
    REQUIRE(state.ir.hi() == 0x44);
    REQUIRE((state.ir.lo() & 0x7f) == 0x55);
    REQUIRE_FALSE(state.iff1);
    REQUIRE(state.iff2);
    REQUIRE(state.int_mode == 0x02);
    REQUIRE(state.pc.get() == 0x8000);
    REQUIRE(state.sp.get() == 0xfffe);
    REQUIRE(bus.port_254 == 0x01);
    REQUIRE(bus.read_data(0x4000) == 0xaa);
    REQUIRE(bus.read_data(0x4001) == 0xbb);
    REQUIRE(bus.read_data(0x4002) == 0xcc);
}

TEST_CASE("Z80 128K snapshots load pages 3 through 10 into physical RAM banks", "[snapshot][z80]") {
    TempFile tmp("jrnz_test_snapshot_128k_pages.z80");
    std::ofstream out(tmp.path, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());

    std::array<uint8_t, 30> header{};
    header[6] = 0x00;
    header[7] = 0x00;
    out.write(reinterpret_cast<const char *>(header.data()), static_cast<std::streamsize>(header.size()));

    const std::array<uint8_t, 25> header2 = {
        0x17, 0x00,              // v2 additional header length
        0x00, 0x90,              // PC
        0x03,                    // original 128K hardware mode
        0x00,                    // last OUT to 0x7ffd, applied in a later loader step
        0x00,                    // Interface 1 ROM paged
        0x00,                    // emulation bits
        0x0e,                    // last AY register index
        0x10, 0x11, 0x12, 0x13,  // AY register contents
        0x14, 0x15, 0x16, 0x17,  //
        0x18, 0x19, 0x1a, 0x1b,  //
        0x1c, 0x1d, 0x5a, 0x1f   //
    };
    out.write(reinterpret_cast<const char *>(header2.data()), static_cast<std::streamsize>(header2.size()));

    for (uint8_t page = 3; page <= 10; ++page) {
        const uint8_t bank = static_cast<uint8_t>(page - 3);
        const std::array<uint8_t, 3> block_header = {0xff, 0xff, page};
        std::array<uint8_t, 0x4000> block{};
        block[0x0000] = static_cast<uint8_t>(0x80 | bank);
        block[0x3fff] = static_cast<uint8_t>(0x40 | bank);

        out.write(reinterpret_cast<const char *>(block_header.data()),
                  static_cast<std::streamsize>(block_header.size()));
        out.write(reinterpret_cast<const char *>(block.data()), static_cast<std::streamsize>(block.size()));
    }
    out.close();

    Bus bus(spectrum_128k_model());
    Z80 state(bus, true);

    bus.load_z80(tmp.path, state);

    REQUIRE(state.pc.get() == 0x9000);
    for (uint8_t bank = 0; bank < 8; ++bank) {
        REQUIRE(bus.read_physical_ram(bank, 0x0000) == static_cast<uint8_t>(0x80 | bank));
        REQUIRE(bus.read_physical_ram(bank, 0x3fff) == static_cast<uint8_t>(0x40 | bank));
    }

    REQUIRE(bus[0x4000] == 0x85);
    REQUIRE(bus[0x8000] == 0x82);
    REQUIRE(bus[0xc000] == 0x80);
}

TEST_CASE("Z80 128K snapshots restore the saved paging register", "[snapshot][z80]") {
    TempFile tmp("jrnz_test_snapshot_128k_paging.z80");
    std::ofstream out(tmp.path, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());

    std::array<uint8_t, 30> header{};
    out.write(reinterpret_cast<const char *>(header.data()), static_cast<std::streamsize>(header.size()));

    const std::array<uint8_t, 25> header2 = {
        0x17, 0x00,              // v2 additional header length
        0x00, 0xa0,              // PC
        0x03,                    // original 128K hardware mode
        0x1b,                    // page RAM 3 at 0xc000, select ROM 1 and shadow screen
        0x00,                    // Interface 1 ROM paged
        0x00,                    // emulation bits
        0x0e,                    // last AY register index
        0x10, 0x11, 0x12, 0x13,  // AY register contents
        0x14, 0x15, 0x16, 0x17,  //
        0x18, 0x19, 0x1a, 0x1b,  //
        0x1c, 0x1d, 0x5a, 0x1f   //
    };
    out.write(reinterpret_cast<const char *>(header2.data()), static_cast<std::streamsize>(header2.size()));

    for (uint8_t page = 3; page <= 10; ++page) {
        const uint8_t bank = static_cast<uint8_t>(page - 3);
        const std::array<uint8_t, 3> block_header = {0xff, 0xff, page};
        std::array<uint8_t, 0x4000> block{};
        block[0x0000] = static_cast<uint8_t>(0x20 | bank);

        out.write(reinterpret_cast<const char *>(block_header.data()),
                  static_cast<std::streamsize>(block_header.size()));
        out.write(reinterpret_cast<const char *>(block.data()), static_cast<std::streamsize>(block.size()));
    }
    out.close();

    Bus bus(spectrum_128k_model());
    Z80 state(bus, true);

    bus.load_z80(tmp.path, state);

    REQUIRE(state.pc.get() == 0xa000);
    REQUIRE(bus.memory_paging_register() == 0x1b);
    REQUIRE(bus.selected_paged_ram_bank() == 3);
    REQUIRE(bus.selected_rom_bank() == 1);
    REQUIRE(bus.shadow_screen_enabled());
    REQUIRE_FALSE(bus.memory_paging_disabled());
    REQUIRE(bus[0x4000] == 0x25);
    REQUIRE(bus[0xc000] == 0x23);
    REQUIRE(bus.read_ula_screen(0x4000) == 0x27);
    REQUIRE(bus.selected_ay_register() == 0x0e);
    REQUIRE(bus.ay_register(0x0e) == 0x5a);
    REQUIRE(bus.read_port(0xfffd) == 0x5a);
}
