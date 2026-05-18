#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("BIT instructions update flags according to the tested bit", "[bitsetres]") {
    SECTION("BIT 0,B clears Z when bit 0 is set and preserves carry") {
        CpuHarness h;
        h.cpu.bc.hi(0x01);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xcb, 0x40});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x01);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("BIT 7,B sets sign when bit 7 is set") {
        CpuHarness h;
        h.cpu.bc.hi(0x80);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xcb, 0x78});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("BIT 7,(HL) sets Z and PV when the tested bit is clear") {
        CpuHarness h;
        h.cpu.hl.set(0x9500);
        h.poke(0x9500, 0x00);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xcb, 0x7e});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("BIT 7,(IX+d) uses the indexed memory byte") {
        CpuHarness h;
        h.cpu.ix.set(0x9601);
        h.poke(0x9600, 0x80);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xdd, 0xcb, 0xff, 0x7e});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 20);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }
}

TEST_CASE("SET instructions modify the addressed bit and preserve flags", "[bitsetres]") {
    SECTION("SET 3,B") {
        CpuHarness h;
        h.cpu.bc.hi(0x00);
        h.cpu.af.flags(0xff);
        h.load({0xcb, 0xd8});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x08);
        REQUIRE(h.cpu.af.flags() == 0xff);
        REQUIRE(h.cpu.pc.get() == 0x0002);
    }

    SECTION("SET 0,(HL)") {
        CpuHarness h;
        h.cpu.hl.set(0x9700);
        h.poke(0x9700, 0x20);
        h.cpu.af.flags(0x2a);
        h.load({0xcb, 0xc6});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(h.peek(0x9700) == 0x21);
        REQUIRE(h.cpu.af.flags() == 0x2a);
        REQUIRE(h.cpu.hl.get() == 0x9700);
    }

    SECTION("SET 0,(IX+d)") {
        CpuHarness h;
        h.cpu.ix.set(0x9800);
        h.poke(0x9801, 0x10);
        h.cpu.af.flags(0x55);
        h.load({0xdd, 0xcb, 0x01, 0xc6});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.peek(0x9801) == 0x11);
        REQUIRE(h.cpu.af.flags() == 0x55);
        REQUIRE(h.cpu.ix.get() == 0x9800);
    }
}

TEST_CASE("RES instructions clear the addressed bit and preserve flags", "[bitsetres]") {
    SECTION("RES 7,B") {
        CpuHarness h;
        h.cpu.bc.hi(0xff);
        h.cpu.af.flags(0x00);
        h.load({0xcb, 0xb8});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x7f);
        REQUIRE(h.cpu.af.flags() == 0x00);
        REQUIRE(h.cpu.pc.get() == 0x0002);
    }

    SECTION("RES 0,(HL)") {
        CpuHarness h;
        h.cpu.hl.set(0x9900);
        h.poke(0x9900, 0xff);
        h.cpu.af.flags(0xa5);
        h.load({0xcb, 0x86});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(h.peek(0x9900) == 0xfe);
        REQUIRE(h.cpu.af.flags() == 0xa5);
        REQUIRE(h.cpu.hl.get() == 0x9900);
    }

    SECTION("RES 0,(IX+d)") {
        CpuHarness h;
        h.cpu.ix.set(0x9a00);
        h.poke(0x99ff, 0x01);
        h.cpu.af.flags(0x5a);
        h.load({0xdd, 0xcb, 0xff, 0x86});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.peek(0x99ff) == 0x00);
        REQUIRE(h.cpu.af.flags() == 0x5a);
        REQUIRE(h.cpu.ix.get() == 0x9a00);
    }
}
