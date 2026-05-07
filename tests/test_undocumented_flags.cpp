#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

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

TEST_CASE("CP follows undocumented F3 and F5 operand bits", "[undocumented][flags][cp]") {
    CpuHarness h;
    h.cpu.af.accum(0x30);
    h.load({0xfe, 0x08});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 7);
    REQUIRE(h.cpu.af.accum() == 0x30);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    require_f3_f5(h, true, false);
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

TEST_CASE("SCF CCF and CPL copy undocumented flag bits from A or the result", "[undocumented][flags][misc]") {
    SECTION("SCF copies F3 and F5 from A") {
        CpuHarness h;
        h.cpu.af.accum(0x28);
        h.cpu.af.flag(RegisterAF::Flags::AddSubtract, true);
        h.cpu.af.flag(RegisterAF::Flags::HalfCarry, true);
        h.cpu.flags_modified_last_instruction = true;
        h.load({0x37});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("SCF ORs in prior F3 and F5 when the previous instruction did not modify flags") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x08);
        h.load({0x40, 0x37});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("SCF takes F3 and F5 directly from A when the previous instruction modified flags") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x08);
        h.load({0xb7, 0x37});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, false, true);
    }

    SECTION("CCF copies F3 and F5 from A while half-carry follows old carry") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x01);
        h.cpu.flags_modified_last_instruction = true;
        h.load({0x3f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }

    SECTION("CCF ORs in prior F3 and F5 when the previous instruction did not modify flags") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x20);
        h.load({0x40, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("CCF takes F3 and F5 directly from A when the previous instruction modified flags") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x20);
        h.load({0xb7, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }

    SECTION("CPL copies F3 and F5 from the complemented result") {
        CpuHarness h;
        h.cpu.af.accum(0xd7);
        h.load({0x2f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x28);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }
}

TEST_CASE("Rotate and shift families copy undocumented F3 and F5 bits from the result",
          "[undocumented][flags][rotate-shift]") {
    SECTION("CB-prefixed rotate copies F3 and F5 from the transformed byte") {
        CpuHarness h;
        h.cpu.bc.hi(0x14);
        h.load({0xcb, 0x00});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x28);
        require_f3_f5(h, true, true);
    }

    SECTION("Accumulator rotate copies F3 and F5 from the new A value while preserving SZPV") {
        CpuHarness h;
        h.cpu.af.accum(0x84);
        h.cpu.af.flags(0xc4);
        h.load({0x07});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x09);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);
    }

    SECTION("Shift copies F3 and F5 from the shifted result") {
        CpuHarness h;
        h.cpu.bc.hi(0x48);
        h.load({0xcb, 0x20});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x90);
        require_f3_f5(h, false, false);
    }
}
