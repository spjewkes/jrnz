#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "test_support.hpp"

TEST_CASE("Indexed CB opcodes update memory and register targets consistently", "[indexed-cb]") {
    SECTION("Rotate and shift forms copy the transformed byte into the selected register") {
        struct IndexedCbCase {
            const char *name;
            std::array<uint8_t, 4> code;
            uint16_t base;
            uint16_t addr;
            uint8_t initial;
            uint8_t expected;
            uint8_t expect_b;
            uint8_t expect_a;
            bool carry;
            bool half;
            bool overflow;
            bool sign;
            bool zero;
        };

        const IndexedCbCase cases[] = {
            {"rlc (ix+d),b",
             {0xdd, 0xcb, 0x01, 0x00},
             0x9800,
             0x9801,
             0x81,
             0x03,
             0x03,
             0x55,
             true,
             false,
             true,
             false,
             false},
            {"srl (iy+d),a",
             {0xfd, 0xcb, 0xfe, 0x3f},
             0x9902,
             0x9900,
             0x81,
             0x40,
             0x12,
             0x40,
             true,
             false,
             false,
             false,
             false},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.bc.hi(0x12);
            h.cpu.af.accum(0x55);
            if (tc.code[0] == 0xdd) {
                h.cpu.ix.set(tc.base);
            } else {
                h.cpu.iy.set(tc.base);
            }
            h.mem.poke_mapped_for_test(tc.addr, tc.initial);
            h.load({tc.code[0], tc.code[1], tc.code[2], tc.code[3]});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 23);
            REQUIRE(step.pc_after == 0x0004);
            REQUIRE(h.mem[tc.addr] == tc.expected);
            REQUIRE(h.cpu.bc.hi() == tc.expect_b);
            REQUIRE(h.cpu.af.accum() == tc.expect_a);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry) == tc.carry);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry) == tc.half);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow) == tc.overflow);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign) == tc.sign);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero) == tc.zero);
            REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        }
    }

    SECTION("SET and RES indexed forms update both memory and the selected register while preserving flags") {
        struct IndexedBitCase {
            const char *name;
            std::array<uint8_t, 4> code;
            uint16_t base;
            uint16_t addr;
            uint8_t initial;
            uint8_t expected;
            uint8_t expect_d;
            uint8_t expect_l;
        };

        const IndexedBitCase cases[] = {
            {"set 2,(ix+d),d", {0xdd, 0xcb, 0xff, 0xd2}, 0x9a01, 0x9a00, 0x10, 0x14, 0x14, 0x44},
            {"res 7,(iy+d),l", {0xfd, 0xcb, 0x02, 0xbd}, 0x9b00, 0x9b02, 0xff, 0x7f, 0x22, 0x7f},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.af.flags(0xa5);
            h.cpu.de.hi(0x22);
            h.cpu.hl.lo(0x44);
            if (tc.code[0] == 0xdd) {
                h.cpu.ix.set(tc.base);
            } else {
                h.cpu.iy.set(tc.base);
            }
            h.mem.poke_mapped_for_test(tc.addr, tc.initial);
            h.load({tc.code[0], tc.code[1], tc.code[2], tc.code[3]});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 23);
            REQUIRE(step.pc_after == 0x0004);
            REQUIRE(h.mem[tc.addr] == tc.expected);
            REQUIRE(h.cpu.de.hi() == tc.expect_d);
            REQUIRE(h.cpu.hl.lo() == tc.expect_l);
            REQUIRE(h.cpu.af.flags() == 0xa5);
        }
    }
}
