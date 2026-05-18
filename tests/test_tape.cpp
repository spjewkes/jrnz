#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "bus.hpp"
#include "tape.hpp"
#include "z80.hpp"

namespace {
class TempTapFile {
public:
    explicit TempTapFile(const char *name) : path(std::string("/tmp/") + name) {}
    ~TempTapFile() { std::remove(path.c_str()); }

    std::string path;
};

std::vector<uint8_t> tap_block(uint8_t flag, std::initializer_list<uint8_t> data) {
    std::vector<uint8_t> bytes{flag};
    bytes.insert(bytes.end(), data.begin(), data.end());

    uint8_t checksum = 0;
    for (uint8_t byte : bytes) {
        checksum ^= byte;
    }
    bytes.push_back(checksum);

    std::vector<uint8_t> block;
    const uint16_t length = static_cast<uint16_t>(bytes.size());
    block.push_back(static_cast<uint8_t>(length & 0xff));
    block.push_back(static_cast<uint8_t>(length >> 8));
    block.insert(block.end(), bytes.begin(), bytes.end());
    return block;
}

void write_bytes(std::ofstream &out, const std::vector<uint8_t> &bytes) {
    out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void set_rom_loader_stack(Bus &bus, Z80 &state, uint16_t return_addr) {
    state.sp.set(0xff00);
    bus.write_addr_to_mem(state.sp.get(), return_addr);
}
}  // namespace

TEST_CASE("TAP parser reads length-prefixed tape blocks", "[tape]") {
    TempTapFile tmp("jrnz_test_tape.tap");
    std::ofstream out(tmp.path, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());
    write_bytes(out, tap_block(0x00, {0x03, 'T', 'E', 'S', 'T'}));
    write_bytes(out, tap_block(0xff, {0x11, 0x22, 0x33}));
    out.close();

    TapeDeck tape;
    REQUIRE(tape.load_tap(tmp.path));

    REQUIRE(tape.has_tape());
    REQUIRE(tape.block_count() == 2);
    REQUIRE(tape.current_block_index() == 0);
}

TEST_CASE("TAP fast-load trap copies a matching ROM data block", "[tape]") {
    TempTapFile tmp("jrnz_test_tape_fast_load.tap");
    std::ofstream out(tmp.path, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());
    write_bytes(out, tap_block(0xff, {0x11, 0x22, 0x33}));
    out.close();

    TapeDeck tape;
    REQUIRE(tape.load_tap(tmp.path));

    Bus bus(65536);
    Z80 state(bus, true);
    state.pc.set(0x0556);
    state.af.accum(0xff);
    state.af.flag(RegisterAF::Flags::Carry, true);
    state.ix.set(0x8000);
    state.de.set(3);
    set_rom_loader_stack(bus, state, 0x1234);

    REQUIRE(tape.try_fast_load(state));

    REQUIRE(state.pc.get() == 0x1234);
    REQUIRE(state.sp.get() == 0xff02);
    REQUIRE(state.ix.get() == 0x8003);
    REQUIRE(state.de.get() == 0);
    REQUIRE(state.af.flag(RegisterAF::Flags::Carry));
    REQUIRE(state.af.accum() == 0);
    REQUIRE(bus[0x8000] == 0x11);
    REQUIRE(bus[0x8001] == 0x22);
    REQUIRE(bus[0x8002] == 0x33);
    REQUIRE(tape.current_block_index() == 1);
}

TEST_CASE("TAP fast-load trap reports failure for a mismatched block flag", "[tape]") {
    TempTapFile tmp("jrnz_test_tape_fast_load_mismatch.tap");
    std::ofstream out(tmp.path, std::ios::binary | std::ios::trunc);
    REQUIRE(out.is_open());
    write_bytes(out, tap_block(0x00, {0x11, 0x22, 0x33}));
    out.close();

    TapeDeck tape;
    REQUIRE(tape.load_tap(tmp.path));

    Bus bus(65536);
    Z80 state(bus, true);
    state.pc.set(0x0556);
    state.af.accum(0xff);
    state.af.flag(RegisterAF::Flags::Carry, true);
    state.ix.set(0x8000);
    state.de.set(3);
    set_rom_loader_stack(bus, state, 0x4567);

    REQUIRE(tape.try_fast_load(state));

    REQUIRE(state.pc.get() == 0x4567);
    REQUIRE_FALSE(state.af.flag(RegisterAF::Flags::Carry));
    REQUIRE(bus[0x8000] == 0x00);
    REQUIRE(tape.current_block_index() == 1);
}
