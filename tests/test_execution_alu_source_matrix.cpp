#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "test_support.hpp"

TEST_CASE("ALU opcodes honor documented source modes across immediate and memory operands", "[alu-matrix]") {
    struct AluSourceCase {
        const char *name;
        std::array<uint8_t, 3> code;
        uint8_t size;
        uint16_t base;
        uint16_t addr;
        uint8_t mem_value;
        uint8_t a;
        bool carry_in;
        uint8_t expected_a;
        bool carry;
        bool half;
        bool overflow;
        bool sign;
        bool zero;
        bool sub;
        uint64_t cycles;
    };

    const AluSourceCase cases[] = {
        {"add a,n",
         {0xc6, 0x01, 0x00},
         2,
         0x0000,
         0x0000,
         0x00,
         0x7f,
         false,
         0x80,
         false,
         true,
         true,
         true,
         false,
         false,
         7},
        {"adc a,(hl)",
         {0x8e, 0x00, 0x00},
         1,
         0x9400,
         0x9400,
         0x00,
         0xff,
         true,
         0x00,
         true,
         true,
         false,
         false,
         true,
         false,
         7},
        {"sub (ix+d)",
         {0xdd, 0x96, 0xfe},
         3,
         0x9504,
         0x9502,
         0x21,
         0x20,
         false,
         0xff,
         true,
         true,
         false,
         true,
         false,
         true,
         19},
        {"sbc a,(iy+d)",
         {0xfd, 0x9e, 0x02},
         3,
         0x9600,
         0x9602,
         0x7f,
         0x80,
         true,
         0x00,
         false,
         true,
         true,
         false,
         true,
         true,
         19},
        {"and (hl)",
         {0xa6, 0x00, 0x00},
         1,
         0x9700,
         0x9700,
         0x0f,
         0xf0,
         false,
         0x00,
         false,
         true,
         true,
         false,
         true,
         false,
         7},
        {"xor (ix+d)",
         {0xdd, 0xae, 0x01},
         3,
         0x9800,
         0x9801,
         0xff,
         0x55,
         false,
         0xaa,
         false,
         false,
         true,
         true,
         false,
         false,
         19},
        {"or (iy+d)",
         {0xfd, 0xb6, 0xff},
         3,
         0x9901,
         0x9900,
         0x80,
         0x01,
         false,
         0x81,
         false,
         false,
         true,
         true,
         false,
         false,
         19},
        {"cp (hl)",
         {0xbe, 0x00, 0x00},
         1,
         0x9a00,
         0x9a00,
         0x40,
         0x40,
         false,
         0x40,
         false,
         false,
         false,
         false,
         true,
         true,
         7},
    };

    for (const auto &tc : cases) {
        CpuHarness h;
        INFO(tc.name);
        h.cpu.af.accum(tc.a);
        h.cpu.af.flags(0x00);
        h.cpu.af.flag(RegisterAF::Flags::Carry, tc.carry_in);
        if (tc.code[0] == 0xdd) {
            h.cpu.ix.set(tc.base);
        } else if (tc.code[0] == 0xfd) {
            h.cpu.iy.set(tc.base);
        } else if (tc.size == 1 && tc.addr != 0x0000) {
            h.cpu.hl.set(tc.base);
        }
        if (tc.addr != 0x0000) {
            h.mem[tc.addr] = tc.mem_value;
        }
        h.load({tc.code[0], tc.code[1], tc.code[2]});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == tc.cycles);
        REQUIRE(step.pc_after == tc.size);
        REQUIRE(h.cpu.af.accum() == tc.expected_a);
        if (tc.addr != 0x0000) {
            REQUIRE(h.mem[tc.addr] == tc.mem_value);
        }
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry) == tc.carry);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry) == tc.half);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow) == tc.overflow);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign) == tc.sign);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero) == tc.zero);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract) == tc.sub);
    }
}
