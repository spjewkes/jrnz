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
