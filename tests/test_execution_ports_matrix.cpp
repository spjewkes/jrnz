#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Register port instructions transfer values through BC-selected ports", "[ports-matrix]") {
    SECTION("IN C,(C) reads the port byte into C and updates flags") {
        CpuHarness h;
        h.cpu.bc.set(0x00fe);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x48});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.bc.lo() == 0xff);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("IN H,(C) uses the odd-port floating bus path") {
        CpuHarness h;
        h.cpu.bc.set(0x0001);
        h.mem.floating_counter = 0;
        h.mem.poke_mapped_for_test(0x4000, 0x81);
        h.cpu.af.flag(RegisterAF::Flags::Carry, false);
        h.load({0xed, 0x60});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.hl.hi() == 0x81);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("OUT (C),B writes the high byte of BC to the selected port") {
        CpuHarness h;
        h.cpu.bc.set(0x44fe);
        h.cpu.af.flags(0xa6);
        h.load({0xed, 0x41});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.mem.port_254 == 0x44);
        REQUIRE(h.cpu.af.flags() == 0xa6);
    }

    SECTION("OUT (C),L writes the low byte of HL through the BC port") {
        CpuHarness h;
        h.cpu.bc.set(0x12fe);
        h.cpu.hl.set(0xab7d);
        h.cpu.af.flags(0x39);
        h.load({0xed, 0x69});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.mem.port_254 == 0x7d);
        REQUIRE(h.cpu.af.flags() == 0x39);
    }
}
