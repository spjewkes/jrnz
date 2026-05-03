#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <string>

#include "bus.hpp"
#include "z80.hpp"

namespace {
class TempSnaFile {
public:
    explicit TempSnaFile(const char *name) : path(std::string("/tmp/") + name) {}
    ~TempSnaFile() { std::remove(path.c_str()); }

    std::string path;
};
}  // namespace

TEST_CASE("SNA snapshot loader restores PC from the stack without disturbing R", "[snapshot][sna]") {
    TempSnaFile tmp("jrnz_test_snapshot.sna");
    std::ofstream out(tmp.path, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());

    const std::array<uint8_t, 27> header = {
        0x44,        // I
        0xa9, 0xcb,  // HL'
        0x65, 0x87,  // DE'
        0x21, 0x43,  // BC'
        0xdd, 0xee,  // AF' = F', A'
        0xbc, 0x9a,  // HL
        0xf0, 0xde,  // DE
        0x78, 0x56,  // BC
        0x34, 0x12,  // IY
        0x78, 0x56,  // IX
        0x04,        // IFF2 set
        0x95,        // R
        0x34, 0x12,  // AF = F, A
        0x00, 0x80,  // SP
        0x02,        // IM 2
        0x03         // border
    };
    out.write(reinterpret_cast<const char *>(header.data()), static_cast<std::streamsize>(header.size()));

    std::array<uint8_t, 49152> ram{};
    ram[0] = 0xaa;
    ram[1] = 0xbb;
    ram[2] = 0xcc;
    ram[0x4000] = 0x34;  // [0x8000] = PC low
    ram[0x4001] = 0x12;  // [0x8001] = PC high
    out.write(reinterpret_cast<const char *>(ram.data()), static_cast<std::streamsize>(ram.size()));
    out.close();

    Bus bus(65536);
    Z80 state(bus, true);
    state.ir.lo(0x00);
    state.iff1 = false;
    state.iff2 = false;

    bus.load_snapshot(tmp.path, state);

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
    REQUIRE(state.ir.lo() == 0x95);
    REQUIRE(state.iff1);
    REQUIRE(state.iff2);
    REQUIRE(state.int_mode == 0x02);
    REQUIRE(state.pc.get() == 0x1234);
    REQUIRE(state.sp.get() == 0x8002);
    REQUIRE(bus.port_254 == 0x03);
    REQUIRE(bus.read_data(0x4000) == 0xaa);
    REQUIRE(bus.read_data(0x4001) == 0xbb);
    REQUIRE(bus.read_data(0x4002) == 0xcc);
}
