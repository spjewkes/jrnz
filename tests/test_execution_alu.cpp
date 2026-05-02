#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Logical immediate opcodes follow Z80 flag rules", "[alu]") {
    SECTION("AND n") {
        CpuHarness h;
        h.cpu.af.accum(0xf0);
        h.cpu.af.flags(0xff);
        h.load({0xe6, 0x0f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 7);
        REQUIRE(h.cpu.af.accum() == 0x00);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }

    SECTION("OR n") {
        CpuHarness h;
        h.cpu.af.accum(0x01);
        h.cpu.af.flags(0xff);
        h.load({0xf6, 0x80});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 7);
        REQUIRE(h.cpu.af.accum() == 0x81);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }

    SECTION("XOR n") {
        CpuHarness h;
        h.cpu.af.accum(0x0f);
        h.cpu.af.flags(0xff);
        h.load({0xee, 0xff});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 7);
        REQUIRE(h.cpu.af.accum() == 0xf0);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }
}

TEST_CASE("CP n compares without modifying A", "[alu]") {
    CpuHarness h;
    h.cpu.af.accum(0x3c);
    h.cpu.af.flags(0x00);
    h.load({0xfe, 0x2f});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 7);
    REQUIRE(h.cpu.af.accum() == 0x3c);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
}

TEST_CASE("SCF CCF and CPL update accumulator and flags per spec", "[alu]") {
    CpuHarness h;
    h.cpu.af.accum(0x35);
    h.cpu.af.flags(0x00);
    h.load({0x37, 0x3f, 0x2f});

    const StepResult scf = h.step();
    REQUIRE(scf.cycle_delta() == 4);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));

    const StepResult ccf = h.step();
    REQUIRE(ccf.cycle_delta() == 4);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));

    const StepResult cpl = h.step();
    REQUIRE(cpl.cycle_delta() == 4);
    REQUIRE(h.cpu.af.accum() == static_cast<uint8_t>(~0x35));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
}

TEST_CASE("DAA adjusts BCD results after addition and subtraction", "[alu]") {
    SECTION("Addition path") {
        CpuHarness h;
        h.cpu.af.accum(0x15);
        h.load({0xc6, 0x27, 0x27});

        h.step();
        REQUIRE(h.cpu.af.accum() == 0x3c);

        const StepResult daa = h.step();
        REQUIRE(daa.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x42);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("Subtraction path") {
        CpuHarness h;
        h.cpu.af.accum(0x15);
        h.load({0xd6, 0x06, 0x27});

        h.step();
        REQUIRE(h.cpu.af.accum() == 0x0f);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));

        const StepResult daa = h.step();
        REQUIRE(daa.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x09);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    }
}
