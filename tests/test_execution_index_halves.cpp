#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("IXH and IXL participate in load instructions as 8-bit registers", "[index-halves]") {
    SECTION("LD IXH,n and LD IXL,n write the high and low bytes of IX") {
        CpuHarness h;
        h.cpu.ix.set(0x0000);
        h.load({0xdd, 0x26, 0x12, 0xdd, 0x2e, 0x34});

        const StepResult high = h.step();
        REQUIRE(high.cycle_delta() == 11);
        REQUIRE(h.cpu.ix.get() == 0x1200);
        REQUIRE(high.pc_after == 0x0003);

        const StepResult low = h.step();
        REQUIRE(low.cycle_delta() == 11);
        REQUIRE(h.cpu.ix.get() == 0x1234);
        REQUIRE(low.pc_after == 0x0006);
    }

    SECTION("LD B,IXH and LD C,IXL read the IX halves") {
        CpuHarness h;
        h.cpu.ix.set(0xabcd);
        h.load({0xdd, 0x44, 0xdd, 0x4d});

        const StepResult into_b = h.step();
        REQUIRE(into_b.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0xab);
        REQUIRE(h.cpu.ix.get() == 0xabcd);

        const StepResult into_c = h.step();
        REQUIRE(into_c.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.lo() == 0xcd);
        REQUIRE(h.cpu.ix.get() == 0xabcd);
    }

    SECTION("LD IXH,B and LD IXL,A update only the targeted half") {
        CpuHarness h;
        h.cpu.ix.set(0x5555);
        h.cpu.bc.hi(0xaa);
        h.cpu.af.accum(0x77);
        h.load({0xdd, 0x60, 0xdd, 0x6f});

        const StepResult from_b = h.step();
        REQUIRE(from_b.cycle_delta() == 8);
        REQUIRE(h.cpu.ix.get() == 0xaa55);
        REQUIRE(h.cpu.bc.hi() == 0xaa);

        const StepResult from_a = h.step();
        REQUIRE(from_a.cycle_delta() == 8);
        REQUIRE(h.cpu.ix.get() == 0xaa77);
        REQUIRE(h.cpu.af.accum() == 0x77);
    }
}

TEST_CASE("IYH and IYL participate in load instructions as 8-bit registers", "[index-halves]") {
    SECTION("LD IYH,n and LD IYL,n write the high and low bytes of IY") {
        CpuHarness h;
        h.cpu.iy.set(0x0000);
        h.load({0xfd, 0x26, 0x9a, 0xfd, 0x2e, 0xbc});

        const StepResult high = h.step();
        REQUIRE(high.cycle_delta() == 11);
        REQUIRE(h.cpu.iy.get() == 0x9a00);
        REQUIRE(high.pc_after == 0x0003);

        const StepResult low = h.step();
        REQUIRE(low.cycle_delta() == 11);
        REQUIRE(h.cpu.iy.get() == 0x9abc);
        REQUIRE(low.pc_after == 0x0006);
    }

    SECTION("LD D,IYH and LD E,IYL read the IY halves") {
        CpuHarness h;
        h.cpu.iy.set(0x2468);
        h.load({0xfd, 0x54, 0xfd, 0x5d});

        const StepResult into_d = h.step();
        REQUIRE(into_d.cycle_delta() == 8);
        REQUIRE(h.cpu.de.hi() == 0x24);
        REQUIRE(h.cpu.iy.get() == 0x2468);

        const StepResult into_e = h.step();
        REQUIRE(into_e.cycle_delta() == 8);
        REQUIRE(h.cpu.de.lo() == 0x68);
        REQUIRE(h.cpu.iy.get() == 0x2468);
    }

    SECTION("LD IYH,D and LD IYL,E update only the targeted half") {
        CpuHarness h;
        h.cpu.iy.set(0x0102);
        h.cpu.de.set(0xa5c3);
        h.load({0xfd, 0x62, 0xfd, 0x6b});

        const StepResult from_d = h.step();
        REQUIRE(from_d.cycle_delta() == 8);
        REQUIRE(h.cpu.iy.get() == 0xa502);
        REQUIRE(h.cpu.de.get() == 0xa5c3);

        const StepResult from_e = h.step();
        REQUIRE(from_e.cycle_delta() == 8);
        REQUIRE(h.cpu.iy.get() == 0xa5c3);
        REQUIRE(h.cpu.de.get() == 0xa5c3);
    }
}

TEST_CASE("IX half-register arithmetic and flag behavior follow 8-bit rules", "[index-halves]") {
    SECTION("ADD A,IXH") {
        CpuHarness h;
        h.cpu.af.accum(0x10);
        h.cpu.ix.set(0x2200);
        h.load({0xdd, 0x84});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.af.accum() == 0x32);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        require_f3_f5(h, false, true);
    }

    SECTION("ADC A,IXL uses incoming carry") {
        CpuHarness h;
        h.cpu.af.accum(0xfe);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.ix.set(0x0001);
        h.load({0xdd, 0x8d});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.af.accum() == 0x00);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("INC IXH preserves carry and updates overflow/sign") {
        CpuHarness h;
        h.cpu.ix.set(0x7f00);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xdd, 0x24});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.ix.get() == 0x8000);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        require_f3_f5(h, false, false);
    }

    SECTION("DEC IXL preserves carry and updates overflow/zero/sign") {
        CpuHarness h;
        h.cpu.ix.set(0x0080);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xdd, 0x2d});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.ix.get() == 0x007f);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }
}

TEST_CASE("IY half-register logic and compare instructions behave as 8-bit operations", "[index-halves]") {
    SECTION("AND IYH") {
        CpuHarness h;
        h.cpu.af.accum(0xf3);
        h.cpu.iy.set(0x5a00);
        h.load({0xfd, 0xa4});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.af.accum() == 0x52);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, false);
    }

    SECTION("XOR IYL") {
        CpuHarness h;
        h.cpu.af.accum(0xff);
        h.cpu.iy.set(0x00f0);
        h.load({0xfd, 0xad});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.af.accum() == 0x0f);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        require_f3_f5(h, true, false);
    }

    SECTION("CP IYH compares without changing A") {
        CpuHarness h;
        h.cpu.af.accum(0x40);
        h.cpu.iy.set(0x4000);
        h.load({0xfd, 0xbc});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.af.accum() == 0x40);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, false, false);
    }
}

TEST_CASE("Indexed half-register arithmetic and compare edge cases match 8-bit flag rules", "[index-halves]") {
    SECTION("IX half-register arithmetic covers half-carry carry and overflow boundaries") {
        struct ArithmeticCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t ix;
            uint8_t a;
            uint8_t initial_flags;
            uint8_t result;
            uint8_t flags;
        };

        const ArithmeticCase cases[] = {
            {"add a,ixl half-carry", {0xdd, 0x85}, 0x0001, 0x0f, 0x00, 0x10, 0x10},
            {"adc a,ixh carry-in to zero", {0xdd, 0x8c}, 0x0000, 0xff, 0x01, 0x00, 0x51},
            {"sub a,ixh half-borrow", {0xdd, 0x94}, 0x0100, 0x10, 0x00, 0x0f, 0x1a},
            {"sbc a,ixl signed overflow", {0xdd, 0x9d}, 0x0000, 0x80, 0x01, 0x7f, 0x3e},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.ix.set(tc.ix);
            h.cpu.af.accum(tc.a);
            h.cpu.af.flags(tc.initial_flags);
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 8);
            REQUIRE(step.pc_after == 0x0002);
            REQUIRE(h.cpu.af.accum() == tc.result);
            require_flags(h.cpu.af.flags(), tc.flags);
        }
    }

    SECTION("IY half-register compare forms preserve A and expose undocumented result bits") {
        struct CompareCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t iy;
            uint8_t a;
            uint8_t flags;
        };

        const CompareCase cases[] = {
            {"cp iyh borrow", {0xfd, 0xbc}, 0x0100, 0x00, 0xbb},
            {"cp iyl equal", {0xfd, 0xbd}, 0x0040, 0x40, 0x42},
            {"cp iyl signed overflow", {0xfd, 0xbd}, 0x00ff, 0x7f, 0xc7},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.iy.set(tc.iy);
            h.cpu.af.accum(tc.a);
            h.cpu.af.flags(0x00);
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 8);
            REQUIRE(step.pc_after == 0x0002);
            REQUIRE(h.cpu.af.accum() == tc.a);
            require_flags(h.cpu.af.flags(), tc.flags);
        }
    }
}
