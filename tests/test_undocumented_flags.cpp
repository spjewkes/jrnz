#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

static void require_f3_f5(const CpuHarness &h, bool f3, bool f5) {
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::F3) == f3);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::F5) == f5);
}

TEST_CASE("Undocumented IN (C) discards the byte but updates flags from the port read", "[undocumented][flags][inc]") {
    SECTION("Even port reads the keyboard or ULA path and preserves general registers") {
        CpuHarness h;
        h.cpu.af.set(0x1243);
        h.cpu.bc.set(0x00fe);
        h.cpu.de.set(0x4567);
        h.cpu.hl.set(0x89ab);
        h.load({0xed, 0x70});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(h.cpu.af.accum() == 0x12);
        REQUIRE(h.cpu.bc.get() == 0x00fe);
        REQUIRE(h.cpu.de.get() == 0x4567);
        REQUIRE(h.cpu.hl.get() == 0x89ab);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        require_f3_f5(h, true, true);
    }

    SECTION("Odd port reads the floating bus and still only updates flags") {
        CpuHarness h;
        h.cpu.af.set(0xaa01);
        h.cpu.bc.set(0x01ff);
        h.cpu.de.set(0x1234);
        h.cpu.hl.set(0x5678);
        h.mem[0x4000] = 0x00;
        h.mem.floating_counter = 0;
        h.load({0xed, 0x70});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(h.cpu.af.accum() == 0xaa);
        REQUIRE(h.cpu.bc.get() == 0x01ff);
        REQUIRE(h.cpu.de.get() == 0x1234);
        REQUIRE(h.cpu.hl.get() == 0x5678);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        require_f3_f5(h, false, false);
    }
}

TEST_CASE("CP follows undocumented F3 and F5 result bits", "[undocumented][flags][cp]") {
    CpuHarness h;
    h.cpu.af.accum(0x30);
    h.load({0xfe, 0x08});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 7);
    REQUIRE(h.cpu.af.accum() == 0x30);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    require_f3_f5(h, true, true);
}

TEST_CASE("BIT copies undocumented flag bits from the tested source", "[undocumented][flags][bit]") {
    SECTION("Register BIT copies F3 and F5 from the tested value") {
        CpuHarness h;
        h.cpu.bc.hi(0x28);
        h.load({0xcb, 0x58});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, true, true);
    }

    SECTION("Indexed BIT copies F3 and F5 from the effective address high byte") {
        CpuHarness h;
        h.cpu.ix.set(0x2834);
        h.mem[0x2834] = 0x00;
        h.load({0xdd, 0xcb, 0x00, 0x46});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 20);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, true);
    }
}

TEST_CASE("LD A,I and LD A,R copy undocumented flag bits from the loaded value", "[undocumented][flags][ir]") {
    SECTION("LD A,I copies F3 and F5 from I") {
        CpuHarness h;
        h.cpu.ir.hi(0x28);
        h.load({0xed, 0x57});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(h.cpu.af.accum() == 0x28);
        require_f3_f5(h, true, true);
    }

    SECTION("LD A,R copies F3 and F5 from the incremented R value") {
        CpuHarness h;
        h.cpu.ir.lo(0x26);
        h.load({0xed, 0x5f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(h.cpu.af.accum() == 0x28);
        require_f3_f5(h, true, true);
    }
}

TEST_CASE("Block compare instructions derive undocumented flags from A minus value minus HF",
          "[undocumented][flags][block-cp]") {
    SECTION("CPI sets F3 from bit 3 and F5 from bit 1 of the adjusted result") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8400);
        h.cpu.bc.set(0x0002);
        h.mem[0x8400] = 0x01;
        h.load({0xed, 0xa1});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.cpu.hl.get() == 0x8401);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, true, false);
    }

    SECTION("CPD uses the same F3 and F5 rule while decrementing HL") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8501);
        h.cpu.bc.set(0x0002);
        h.mem[0x8501] = 0x01;
        h.load({0xed, 0xa9});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.cpu.hl.get() == 0x8500);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);
    }

    SECTION("CPIR keeps the adjusted F3 and F5 values on the repeating step") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8600);
        h.cpu.bc.set(0x0002);
        h.mem[0x8600] = 0x01;
        h.mem[0x8601] = 0x09;
        h.load({0xed, 0xb1});

        const StepResult first = h.step();

        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.hl.get() == 0x8601);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);

        const StepResult second = h.step();

        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x8602);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, false);
    }

    SECTION("CPDR keeps the adjusted F3 and F5 values on the repeating step") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8701);
        h.cpu.bc.set(0x0002);
        h.mem[0x8701] = 0x01;
        h.mem[0x8700] = 0x09;
        h.load({0xed, 0xb9});

        const StepResult first = h.step();

        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.hl.get() == 0x8700);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);

        const StepResult second = h.step();

        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x86ff);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, false);
    }
}
