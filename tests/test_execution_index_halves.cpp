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

        const StepResult low = h.step();
        REQUIRE(low.cycle_delta() == 11);
        REQUIRE(h.cpu.ix.get() == 0x1234);
    }

    SECTION("LD B,IXH and LD C,IXL read the IX halves") {
        CpuHarness h;
        h.cpu.ix.set(0xabcd);
        h.load({0xdd, 0x44, 0xdd, 0x4d});

        const StepResult into_b = h.step();
        REQUIRE(into_b.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0xab);

        const StepResult into_c = h.step();
        REQUIRE(into_c.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.lo() == 0xcd);
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

        const StepResult from_a = h.step();
        REQUIRE(from_a.cycle_delta() == 8);
        REQUIRE(h.cpu.ix.get() == 0xaa77);
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

        const StepResult low = h.step();
        REQUIRE(low.cycle_delta() == 11);
        REQUIRE(h.cpu.iy.get() == 0x9abc);
    }

    SECTION("LD D,IYH and LD E,IYL read the IY halves") {
        CpuHarness h;
        h.cpu.iy.set(0x2468);
        h.load({0xfd, 0x54, 0xfd, 0x5d});

        const StepResult into_d = h.step();
        REQUIRE(into_d.cycle_delta() == 8);
        REQUIRE(h.cpu.de.hi() == 0x24);

        const StepResult into_e = h.step();
        REQUIRE(into_e.cycle_delta() == 8);
        REQUIRE(h.cpu.de.lo() == 0x68);
    }

    SECTION("LD IYH,D and LD IYL,E update only the targeted half") {
        CpuHarness h;
        h.cpu.iy.set(0x0102);
        h.cpu.de.set(0xa5c3);
        h.load({0xfd, 0x62, 0xfd, 0x6b});

        const StepResult from_d = h.step();
        REQUIRE(from_d.cycle_delta() == 8);
        REQUIRE(h.cpu.iy.get() == 0xa502);

        const StepResult from_e = h.step();
        REQUIRE(from_e.cycle_delta() == 8);
        REQUIRE(h.cpu.iy.get() == 0xa5c3);
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
    }
}
