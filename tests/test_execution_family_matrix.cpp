#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "test_support.hpp"

TEST_CASE("Base LD family opcodes transfer values across registers and memory consistently", "[family-matrix]") {
    struct LoadCase {
        const char *name;
        std::array<uint8_t, 2> code;
        uint8_t size;
        uint8_t b;
        uint8_t c;
        uint8_t d;
        uint8_t e;
        uint8_t h;
        uint8_t l;
        uint8_t a;
        uint16_t hl_addr;
        uint8_t mem_value;
        uint8_t expect_b;
        uint8_t expect_c;
        uint8_t expect_d;
        uint8_t expect_e;
        uint8_t expect_h;
        uint8_t expect_l;
        uint8_t expect_a;
        uint8_t expect_mem;
        uint64_t cycles;
    };

    const LoadCase cases[] = {
        {"ld b,d", {0x42, 0x00}, 1,    0x10, 0x20, 0x33, 0x40, 0x50, 0x60, 0x70, 0x9400,
         0xaa,     0x33,         0x20, 0x33, 0x40, 0x94, 0x00, 0x70, 0xaa, 4},
        {"ld c,a", {0x4f, 0x00}, 1,    0x10, 0x20, 0x30, 0x40, 0x94, 0x03, 0xe1, 0x9403,
         0x55,     0x10,         0xe1, 0x30, 0x40, 0x94, 0x03, 0xe1, 0x55, 4},
        {"ld e,(hl)", {0x5e, 0x00}, 1,    0x10, 0x20, 0x30, 0x40, 0x94, 0x00, 0x70, 0x9400,
         0xab,        0x10,         0x20, 0x30, 0xab, 0x94, 0x00, 0x70, 0xab, 7},
        {"ld h,c", {0x61, 0x00}, 1,    0x10, 0x27, 0x30, 0x40, 0x50, 0x60, 0x70, 0x9404,
         0x13,     0x10,         0x27, 0x30, 0x40, 0x27, 0x04, 0x70, 0x13, 4},
        {"ld (hl),a", {0x77, 0x00}, 1,    0x10, 0x20, 0x30, 0x40, 0x94, 0x01, 0xcd, 0x9401,
         0x12,        0x10,         0x20, 0x30, 0x40, 0x94, 0x01, 0xcd, 0xcd, 7},
        {"ld a,l", {0x7d, 0x00}, 1,    0x10, 0x20, 0x30, 0x40, 0x50, 0x6e, 0x00, 0x9402,
         0x12,     0x10,         0x20, 0x30, 0x40, 0x94, 0x02, 0x02, 0x12, 4},
        {"ld a,(hl)", {0x7e, 0x00}, 1,    0x10, 0x20, 0x30, 0x40, 0x94, 0x05, 0x00, 0x9405,
         0x9c,        0x10,         0x20, 0x30, 0x40, 0x94, 0x05, 0x9c, 0x9c, 7},
    };

    for (const auto &tc : cases) {
        CpuHarness h;
        INFO(tc.name);
        h.cpu.af.flags(0xa5);
        h.cpu.bc.set(static_cast<uint16_t>((tc.b << 8) | tc.c));
        h.cpu.de.set(static_cast<uint16_t>((tc.d << 8) | tc.e));
        h.cpu.hl.set(tc.hl_addr);
        h.cpu.af.accum(tc.a);
        h.poke(tc.hl_addr, tc.mem_value);
        h.load({tc.code[0], tc.code[1]});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == tc.cycles);
        REQUIRE(step.pc_after == tc.size);
        REQUIRE(h.cpu.bc.hi() == tc.expect_b);
        REQUIRE(h.cpu.bc.lo() == tc.expect_c);
        REQUIRE(h.cpu.de.hi() == tc.expect_d);
        REQUIRE(h.cpu.de.lo() == tc.expect_e);
        REQUIRE(h.cpu.hl.hi() == tc.expect_h);
        REQUIRE(h.cpu.hl.lo() == tc.expect_l);
        REQUIRE(h.cpu.af.accum() == tc.expect_a);
        REQUIRE(h.peek(tc.hl_addr) == tc.expect_mem);
        REQUIRE(h.cpu.af.flags() == 0xa5);
    }
}

TEST_CASE("Base ALU register opcodes apply the selected source operand consistently", "[family-matrix]") {
    struct AluCase {
        const char *name;
        uint8_t opcode;
        uint8_t a;
        uint8_t src;
        bool carry_in;
        uint8_t expect_a;
        bool carry;
        bool half;
        bool overflow;
        bool sign;
        bool zero;
        bool sub;
    };

    const AluCase cases[] = {
        {"add a,b", 0x80, 0x10, 0x22, false, 0x32, false, false, false, false, false, false},
        {"adc a,b", 0x88, 0xfe, 0x01, true, 0x00, true, true, false, false, true, false},
        {"add a,a", 0x87, 0x40, 0x00, false, 0x80, false, false, true, true, false, false},
        {"sub b", 0x90, 0x10, 0x01, false, 0x0f, false, true, false, false, false, true},
        {"sbc a,b", 0x98, 0x00, 0x00, true, 0xff, true, true, false, true, false, true},
        {"and b", 0xa0, 0xf0, 0x0f, false, 0x00, false, true, true, false, true, false},
        {"and a", 0xa7, 0x81, 0x00, false, 0x81, false, true, true, true, false, false},
        {"xor b", 0xa8, 0xff, 0x0f, false, 0xf0, false, false, true, true, false, false},
        {"xor a", 0xaf, 0x5a, 0x00, false, 0x00, false, false, true, false, true, false},
        {"or b", 0xb0, 0x00, 0x80, false, 0x80, false, false, false, true, false, false},
        {"or a", 0xb7, 0x00, 0x00, false, 0x00, false, false, true, false, true, false},
        {"cp b", 0xb8, 0x40, 0x40, false, 0x40, false, false, false, false, true, true},
        {"cp a", 0xbf, 0x91, 0x00, false, 0x91, false, false, false, false, true, true},
    };

    for (const auto &tc : cases) {
        CpuHarness h;
        INFO(tc.name);
        h.cpu.af.accum(tc.a);
        h.cpu.af.flags(0x00);
        h.cpu.af.flag(RegisterAF::Flags::Carry, tc.carry_in);
        h.cpu.bc.hi(tc.src);
        h.load({tc.opcode});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(step.pc_after == 0x0001);
        REQUIRE(h.cpu.af.accum() == tc.expect_a);
        REQUIRE(h.cpu.bc.hi() == tc.src);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry) == tc.carry);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry) == tc.half);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow) == tc.overflow);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign) == tc.sign);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero) == tc.zero);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract) == tc.sub);
    }
}
