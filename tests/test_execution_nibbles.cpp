#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("RLD and RRD rotate nibbles between A and (HL) with documented flags", "[nibbles]") {
    SECTION("RLD rotates A low nibble into memory high nibble") {
        CpuHarness h;
        h.cpu.af.accum(0x3c);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.hl.set(0x9300);
        h.mem.poke_mapped_for_test(0x9300, 0xa5);
        h.load({0xed, 0x6f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 18);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0x3a);
        REQUIRE(h.mem[0x9300] == 0x5c);
        REQUIRE(h.cpu.hl.get() == 0x9300);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }

    SECTION("RRD rotates memory low nibble into A low nibble") {
        CpuHarness h;
        h.cpu.af.accum(0x40);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.hl.set(0x9301);
        h.mem.poke_mapped_for_test(0x9301, 0x12);
        h.load({0xed, 0x67});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 18);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0x42);
        REQUIRE(h.mem[0x9301] == 0x01);
        REQUIRE(h.cpu.hl.get() == 0x9301);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }

    SECTION("RRD sets zero and parity from the updated accumulator") {
        CpuHarness h;
        h.cpu.af.accum(0x00);
        h.cpu.af.flag(RegisterAF::Flags::Carry, false);
        h.cpu.hl.set(0x9302);
        h.mem.poke_mapped_for_test(0x9302, 0x00);
        h.load({0xed, 0x67});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 18);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0x00);
        REQUIRE(h.mem[0x9302] == 0x00);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    }
}
