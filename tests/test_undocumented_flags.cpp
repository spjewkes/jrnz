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
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::F3));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::F5));
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
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::F3));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::F5));
    }
}
