#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("LDI copies a byte and updates registers and flags", "[extended]") {
    CpuHarness h;
    h.cpu.hl.set(0x5000);
    h.cpu.de.set(0x6000);
    h.cpu.bc.set(0x0002);
    h.cpu.af.flag(RegisterAF::Flags::Carry, true);
    h.mem[0x5000] = 0xa5;
    h.load({0xed, 0xa0});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 16);
    REQUIRE(h.mem[0x6000] == 0xa5);
    REQUIRE(h.cpu.hl.get() == 0x5001);
    REQUIRE(h.cpu.de.get() == 0x6001);
    REQUIRE(h.cpu.bc.get() == 0x0001);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
}

TEST_CASE("LDIR repeats until BC becomes zero", "[extended]") {
    CpuHarness h;
    h.cpu.hl.set(0x5000);
    h.cpu.de.set(0x6000);
    h.cpu.bc.set(0x0002);
    h.mem[0x5000] = 0x12;
    h.mem[0x5001] = 0x34;
    h.load({0xed, 0xb0});

    const StepResult first = h.step();
    REQUIRE(first.cycle_delta() == 21);
    REQUIRE(h.cpu.pc.get() == 0x0000);
    REQUIRE(h.mem[0x6000] == 0x12);
    REQUIRE(h.cpu.hl.get() == 0x5001);
    REQUIRE(h.cpu.de.get() == 0x6001);
    REQUIRE(h.cpu.bc.get() == 0x0001);

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.mem[0x6001] == 0x34);
    REQUIRE(h.cpu.hl.get() == 0x5002);
    REQUIRE(h.cpu.de.get() == 0x6002);
    REQUIRE(h.cpu.bc.get() == 0x0000);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
}

TEST_CASE("CPIR repeats until match and leaves HL and BC in the matched state", "[extended]") {
    CpuHarness h;
    h.cpu.af.accum(0x34);
    h.cpu.hl.set(0x5200);
    h.cpu.bc.set(0x0003);
    h.mem[0x5200] = 0x12;
    h.mem[0x5201] = 0x34;
    h.mem[0x5202] = 0x56;
    h.load({0xed, 0xb1});

    const StepResult first = h.step();
    REQUIRE(first.cycle_delta() == 21);
    REQUIRE(h.cpu.pc.get() == 0x0000);
    REQUIRE(h.cpu.hl.get() == 0x5201);
    REQUIRE(h.cpu.bc.get() == 0x0002);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.cpu.hl.get() == 0x5202);
    REQUIRE(h.cpu.bc.get() == 0x0001);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
}

TEST_CASE("Indexed addressing uses signed displacement with IX and IY", "[indexed]") {
    SECTION("LD (IX+d),n then LD A,(IX+d)") {
        CpuHarness h;
        h.cpu.ix.set(0x7004);
        h.load({0xdd, 0x36, 0xfe, 0x9a, 0xdd, 0x7e, 0xfe});

        const StepResult store = h.step();
        REQUIRE(store.cycle_delta() == 19);
        REQUIRE(h.mem[0x7002] == 0x9a);

        const StepResult load = h.step();
        REQUIRE(load.cycle_delta() == 19);
        REQUIRE(h.cpu.af.accum() == 0x9a);
    }

    SECTION("LD (IY+d),n with positive displacement") {
        CpuHarness h;
        h.cpu.iy.set(0x7100);
        h.load({0xfd, 0x36, 0x05, 0x33});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 19);
        REQUIRE(h.mem[0x7105] == 0x33);
    }
}

TEST_CASE("BIT on indexed memory updates flags from the fetched byte", "[indexed]") {
    CpuHarness h;
    h.cpu.ix.set(0x7200);
    h.cpu.af.flag(RegisterAF::Flags::Carry, true);
    h.mem[0x71ff] = 0x80;
    h.load({0xdd, 0xcb, 0xff, 0x7e});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 20);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
}

TEST_CASE("R register increments according to fetched opcode length", "[rreg]") {
    SECTION("Base opcode increments R by one") {
        CpuHarness h;
        h.cpu.ir.lo(0x00);
        h.load({0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x01);
    }

    SECTION("CB prefix increments R by two") {
        CpuHarness h;
        h.cpu.bc.hi(0x01);
        h.cpu.ir.lo(0x00);
        h.load({0xcb, 0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x02);
    }

    SECTION("Ignored DD prefix still increments R by two") {
        CpuHarness h;
        h.cpu.ir.lo(0x00);
        h.load({0xdd, 0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x02);
        REQUIRE(h.cpu.pc.get() == 0x0002);
    }

    SECTION("DDCB indexed bit op increments R by three") {
        CpuHarness h;
        h.cpu.ix.set(0x7300);
        h.cpu.ir.lo(0x00);
        h.mem[0x7300] = 0x01;
        h.load({0xdd, 0xcb, 0x00, 0x46});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x03);
    }
}
