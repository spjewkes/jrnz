#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("8-bit arithmetic instructions follow documented result and flag rules", "[arith16]") {
    SECTION("ADD A,(HL)") {
        CpuHarness h;
        h.cpu.af.accum(0x7f);
        h.cpu.hl.set(0x8400);
        h.mem.poke_mapped_for_test(0x8400, 0x01);
        h.load({0x86});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 7);
        REQUIRE(step.pc_after == 0x0001);
        REQUIRE(h.cpu.af.accum() == 0x80);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("SUB n") {
        CpuHarness h;
        h.cpu.af.accum(0x10);
        h.load({0xd6, 0x01});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 7);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0x0f);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("SBC A,(HL) uses incoming carry") {
        CpuHarness h;
        h.cpu.af.accum(0x00);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.hl.set(0x8410);
        h.mem.poke_mapped_for_test(0x8410, 0x00);
        h.load({0x9e});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 7);
        REQUIRE(step.pc_after == 0x0001);
        REQUIRE(h.cpu.af.accum() == 0xff);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("ADD A,(IX+d) uses indexed displacement") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.ix.set(0x8504);
        h.mem.poke_mapped_for_test(0x8502, 0x22);
        h.load({0xdd, 0x86, 0xfe});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 19);
        REQUIRE(step.pc_after == 0x0003);
        REQUIRE(h.cpu.af.accum() == 0x42);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("SBC A,(IY+d) subtracts indexed memory and borrow") {
        CpuHarness h;
        h.cpu.af.accum(0x40);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.iy.set(0x8600);
        h.mem.poke_mapped_for_test(0x8602, 0x41);
        h.load({0xfd, 0x9e, 0x02});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 19);
        REQUIRE(step.pc_after == 0x0003);
        REQUIRE(h.cpu.af.accum() == 0xfe);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }
}

TEST_CASE("16-bit arithmetic instructions follow documented carry and overflow rules", "[arith16]") {
    SECTION("ADD HL,DE preserves N and updates carry and half-carry") {
        CpuHarness h;
        h.cpu.hl.set(0x8fff);
        h.cpu.de.set(0x0001);
        h.cpu.af.flag(RegisterAF::Flags::AddSubtract, true);
        h.load({0x19});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 11);
        REQUIRE(step.pc_after == 0x0001);
        REQUIRE(h.cpu.hl.get() == 0x9000);
        REQUIRE(h.cpu.de.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("ADC HL,BC sets overflow on signed wrap") {
        CpuHarness h;
        h.cpu.hl.set(0x7fff);
        h.cpu.bc.set(0x0001);
        h.cpu.af.flags(0x00);
        h.load({0xed, 0x4a});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x8000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("ADC HL,DE uses bit 11 for half-carry and includes incoming carry") {
        CpuHarness h;
        h.cpu.hl.set(0x0f00);
        h.cpu.de.set(0x00ff);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x5a});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x1000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("SBC HL,SP uses incoming carry and sets zero") {
        CpuHarness h;
        h.cpu.hl.set(0x1001);
        h.cpu.sp.set(0x1000);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x72});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("SBC HL,BC includes incoming carry in half-borrow") {
        CpuHarness h;
        h.cpu.hl.set(0x1000);
        h.cpu.bc.set(0x00ff);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x42});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x0f00);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("ADD IY,SP updates the indexed register pair") {
        CpuHarness h;
        h.cpu.iy.set(0xffff);
        h.cpu.sp.set(0x0001);
        h.load({0xfd, 0x39});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.iy.get() == 0x0000);
        REQUIRE(h.cpu.sp.get() == 0x0001);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }
}
