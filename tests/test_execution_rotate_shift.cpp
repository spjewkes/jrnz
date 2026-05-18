#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("CB rotate instructions update register values and flags", "[rotate-shift]") {
    SECTION("RLC B rotates bit 7 into carry and bit 0") {
        CpuHarness h;
        h.cpu.bc.hi(0x81);
        h.cpu.af.flags(0x00);
        h.load({0xcb, 0x00});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x03);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }

    SECTION("RL B rotates old carry into bit 0") {
        CpuHarness h;
        h.cpu.bc.hi(0x80);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xcb, 0x10});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x01);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("RRC B rotates bit 0 into carry and bit 7") {
        CpuHarness h;
        h.cpu.bc.hi(0x01);
        h.cpu.af.flags(0x00);
        h.load({0xcb, 0x08});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x80);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }

    SECTION("RR B rotates old carry into bit 7") {
        CpuHarness h;
        h.cpu.bc.hi(0x01);
        h.cpu.af.flag(RegisterAF::Flags::Carry, false);
        h.load({0xcb, 0x18});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x00);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }
}

TEST_CASE("Shift instructions follow Z80 result and flag rules", "[rotate-shift]") {
    SECTION("SLA B shifts left and clears bit 0") {
        CpuHarness h;
        h.cpu.bc.hi(0xc1);
        h.load({0xcb, 0x20});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x82);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }

    SECTION("SRA B preserves the original sign bit") {
        CpuHarness h;
        h.cpu.bc.hi(0x81);
        h.load({0xcb, 0x28});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0xc0);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }

    SECTION("SRL B shifts in zero at bit 7") {
        CpuHarness h;
        h.cpu.bc.hi(0x81);
        h.load({0xcb, 0x38});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x40);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }
}

TEST_CASE("Accumulator rotate instructions preserve SZPV and update carry", "[rotate-shift]") {
    SECTION("RLCA rotates left circular through A only") {
        CpuHarness h;
        h.cpu.af.accum(0x81);
        h.cpu.af.flag(RegisterAF::Flags::Zero, true);
        h.cpu.af.flag(RegisterAF::Flags::Sign, true);
        h.cpu.af.flag(RegisterAF::Flags::ParityOverflow, true);
        h.load({0x07});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x03);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }

    SECTION("RLA rotates old carry into A bit 0") {
        CpuHarness h;
        h.cpu.af.accum(0x80);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.af.flag(RegisterAF::Flags::Zero, true);
        h.cpu.af.flag(RegisterAF::Flags::Sign, false);
        h.cpu.af.flag(RegisterAF::Flags::ParityOverflow, false);
        h.load({0x17});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x01);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("RRCA rotates right circular through A only") {
        CpuHarness h;
        h.cpu.af.accum(0x01);
        h.cpu.af.flag(RegisterAF::Flags::Zero, false);
        h.cpu.af.flag(RegisterAF::Flags::Sign, false);
        h.cpu.af.flag(RegisterAF::Flags::ParityOverflow, true);
        h.load({0x0f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x80);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }

    SECTION("RRA rotates old carry into A bit 7") {
        CpuHarness h;
        h.cpu.af.accum(0x01);
        h.cpu.af.flag(RegisterAF::Flags::Carry, false);
        h.cpu.af.flag(RegisterAF::Flags::Zero, false);
        h.cpu.af.flag(RegisterAF::Flags::Sign, true);
        h.cpu.af.flag(RegisterAF::Flags::ParityOverflow, false);
        h.load({0x1f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x00);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }
}

TEST_CASE("Rotate and shift instructions operate on memory through HL", "[rotate-shift]") {
    SECTION("RLC (HL)") {
        CpuHarness h;
        h.cpu.hl.set(0x9400);
        h.mem.poke_mapped_for_test(0x9400, 0x80);
        h.load({0xcb, 0x06});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(h.mem[0x9400] == 0x01);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("SRL (HL)") {
        CpuHarness h;
        h.cpu.hl.set(0x9401);
        h.mem.poke_mapped_for_test(0x9401, 0x01);
        h.load({0xcb, 0x3e});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(h.mem[0x9401] == 0x00);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }
}
