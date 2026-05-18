#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "test_support.hpp"

TEST_CASE("16-bit pair and increment-decrement opcodes update targets consistently", "[matrix-16bit]") {
    SECTION("Representative 16-bit INC and DEC opcodes update register pairs without touching flags") {
        struct PairCase {
            const char *name;
            uint8_t opcode;
            uint16_t initial;
            uint16_t expected;
        };

        const PairCase cases[] = {
            {"inc bc", 0x03, 0x1234, 0x1235},
            {"dec de", 0x1b, 0x4567, 0x4566},
            {"inc hl", 0x23, 0x89ab, 0x89ac},
            {"dec sp", 0x3b, 0x0000, 0xffff},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.af.flags(0xa5);
            switch (tc.opcode) {
                case 0x03:
                    h.cpu.bc.set(tc.initial);
                    break;
                case 0x1b:
                    h.cpu.de.set(tc.initial);
                    break;
                case 0x23:
                    h.cpu.hl.set(tc.initial);
                    break;
                case 0x3b:
                    h.cpu.sp.set(tc.initial);
                    break;
            }
            h.load({tc.opcode});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 6);
            REQUIRE(step.pc_after == 0x0001);
            switch (tc.opcode) {
                case 0x03:
                    REQUIRE(h.cpu.bc.get() == tc.expected);
                    break;
                case 0x1b:
                    REQUIRE(h.cpu.de.get() == tc.expected);
                    break;
                case 0x23:
                    REQUIRE(h.cpu.hl.get() == tc.expected);
                    break;
                case 0x3b:
                    REQUIRE(h.cpu.sp.get() == tc.expected);
                    break;
            }
            REQUIRE(h.cpu.af.flags() == 0xa5);
        }
    }

    SECTION("Representative 8-bit INC and DEC opcodes use the selected target and preserve carry") {
        struct ByteCase {
            const char *name;
            uint8_t opcode;
            uint8_t initial;
            uint8_t expected;
            bool zero;
            bool sign;
            bool overflow;
            bool half;
            bool sub;
        };

        const ByteCase cases[] = {
            {"inc b", 0x04, 0x7f, 0x80, false, true, true, true, false},
            {"dec c", 0x0d, 0x00, 0xff, false, true, false, true, true},
            {"inc (hl)", 0x34, 0xff, 0x00, true, false, false, true, false},
            {"dec a", 0x3d, 0x80, 0x7f, false, false, true, true, true},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.af.flag(RegisterAF::Flags::Carry, true);
            if (tc.opcode == 0x34) {
                h.cpu.hl.set(0x9500);
                h.poke(0x9500, tc.initial);
            } else if (tc.opcode == 0x04) {
                h.cpu.bc.hi(tc.initial);
            } else if (tc.opcode == 0x0d) {
                h.cpu.bc.lo(tc.initial);
            } else if (tc.opcode == 0x3d) {
                h.cpu.af.accum(tc.initial);
            }
            h.load({tc.opcode});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == (tc.opcode == 0x34 ? 11 : 4));
            REQUIRE(step.pc_after == 0x0001);
            if (tc.opcode == 0x34) {
                REQUIRE(h.peek(0x9500) == tc.expected);
            } else if (tc.opcode == 0x04) {
                REQUIRE(h.cpu.bc.hi() == tc.expected);
            } else if (tc.opcode == 0x0d) {
                REQUIRE(h.cpu.bc.lo() == tc.expected);
            } else if (tc.opcode == 0x3d) {
                REQUIRE(h.cpu.af.accum() == tc.expected);
            }
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero) == tc.zero);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign) == tc.sign);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow) == tc.overflow);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry) == tc.half);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract) == tc.sub);
        }
    }
}

TEST_CASE("Indexed and extended 16-bit arithmetic opcodes honor their selected operands", "[matrix-16bit]") {
    SECTION("Indexed pair adds update IX and IY with the selected source pair") {
        struct AddCase {
            const char *name;
            std::array<uint8_t, 2> code;
            uint16_t initial;
            uint16_t rhs;
            uint16_t expected;
            bool carry;
            bool half;
        };

        const AddCase cases[] = {
            {"add ix,bc", {0xdd, 0x09}, 0x8fff, 0x0001, 0x9000, false, true},
            {"add iy,sp", {0xfd, 0x39}, 0xffff, 0x0001, 0x0000, true, true},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.af.flags(0x00);
            if (tc.code[0] == 0xdd) {
                h.cpu.ix.set(tc.initial);
                h.cpu.bc.set(tc.rhs);
            } else {
                h.cpu.iy.set(tc.initial);
                h.cpu.sp.set(tc.rhs);
            }
            h.load({tc.code[0], tc.code[1]});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 15);
            REQUIRE(step.pc_after == 0x0002);
            if (tc.code[0] == 0xdd) {
                REQUIRE(h.cpu.ix.get() == tc.expected);
            } else {
                REQUIRE(h.cpu.iy.get() == tc.expected);
            }
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry) == tc.carry);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry) == tc.half);
            REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        }
    }

    SECTION("Indexed INC and DEC memory forms use the displacement and update flags") {
        struct IndexedMemCase {
            const char *name;
            std::array<uint8_t, 3> code;
            uint16_t base;
            uint16_t addr;
            uint8_t initial;
            uint8_t expected;
            bool zero;
            bool sign;
            bool overflow;
            bool half;
            bool sub;
        };

        const IndexedMemCase cases[] = {
            {"inc (ix+d)", {0xdd, 0x34, 0xff}, 0x9601, 0x9600, 0x7f, 0x80, false, true, true, true, false},
            {"dec (iy+d)", {0xfd, 0x35, 0x02}, 0x9700, 0x9702, 0x01, 0x00, true, false, false, false, true},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.af.flag(RegisterAF::Flags::Carry, true);
            if (tc.code[0] == 0xdd) {
                h.cpu.ix.set(tc.base);
            } else {
                h.cpu.iy.set(tc.base);
            }
            h.poke(tc.addr, tc.initial);
            h.load({tc.code[0], tc.code[1], tc.code[2]});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 23);
            REQUIRE(step.pc_after == 0x0003);
            REQUIRE(h.peek(tc.addr) == tc.expected);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero) == tc.zero);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign) == tc.sign);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow) == tc.overflow);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry) == tc.half);
            REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract) == tc.sub);
        }
    }

    SECTION("Extended ADC HL,rr and SBC HL,rr forms use the selected source pair") {
        CpuHarness h;
        h.cpu.hl.set(0x1000);
        h.cpu.de.set(0x0fff);
        h.cpu.af.flags(0x00);
        h.load({0xed, 0x5a, 0xed, 0x52});

        const StepResult adc = h.step();
        REQUIRE(adc.cycle_delta() == 15);
        REQUIRE(adc.pc_after == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x1fff);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));

        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        const StepResult sbc = h.step();
        REQUIRE(sbc.cycle_delta() == 15);
        REQUIRE(sbc.pc_after == 0x0004);
        REQUIRE(h.cpu.hl.get() == 0x0fff);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }
}
