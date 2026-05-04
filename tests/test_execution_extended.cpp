#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("LDI copies a byte and updates registers and flags", "[extended]") {
    CpuHarness h;
    h.cpu.af.accum(0x11);
    h.cpu.hl.set(0x5000);
    h.cpu.de.set(0x6000);
    h.cpu.bc.set(0x0002);
    h.cpu.af.flag(RegisterAF::Flags::Carry, true);
    h.mem[0x5000] = 0xa5;
    h.load({0xed, 0xa0});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 16);
    REQUIRE(step.pc_after == 0x0002);
    REQUIRE(h.mem[0x6000] == 0xa5);
    REQUIRE(h.cpu.hl.get() == 0x5001);
    REQUIRE(h.cpu.de.get() == 0x6001);
    REQUIRE(h.cpu.bc.get() == 0x0001);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE(h.cpu.af.accum() == 0x11);
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
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.mem[0x6001] == 0x34);
    REQUIRE(h.cpu.hl.get() == 0x5002);
    REQUIRE(h.cpu.de.get() == 0x6002);
    REQUIRE(h.cpu.bc.get() == 0x0000);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
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
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE(h.cpu.af.accum() == 0x34);

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.cpu.hl.get() == 0x5202);
    REQUIRE(h.cpu.bc.get() == 0x0001);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE(h.cpu.af.accum() == 0x34);
}

TEST_CASE("Indexed addressing uses signed displacement with IX and IY", "[indexed]") {
    SECTION("LD (IX+d),n then LD A,(IX+d)") {
        CpuHarness h;
        h.cpu.ix.set(0x7004);
        h.load({0xdd, 0x36, 0xfe, 0x9a, 0xdd, 0x7e, 0xfe});

        const StepResult store = h.step();
        REQUIRE(store.cycle_delta() == 19);
        REQUIRE(store.pc_after == 0x0004);
        REQUIRE(h.mem[0x7002] == 0x9a);
        REQUIRE(h.cpu.ix.get() == 0x7004);

        const StepResult load = h.step();
        REQUIRE(load.cycle_delta() == 19);
        REQUIRE(load.pc_after == 0x0007);
        REQUIRE(h.cpu.af.accum() == 0x9a);
        REQUIRE(h.cpu.ix.get() == 0x7004);
    }

    SECTION("LD (IY+d),n with positive displacement") {
        CpuHarness h;
        h.cpu.iy.set(0x7100);
        h.load({0xfd, 0x36, 0x05, 0x33});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 19);
        REQUIRE(step.pc_after == 0x0004);
        REQUIRE(h.mem[0x7105] == 0x33);
        REQUIRE(h.cpu.iy.get() == 0x7100);
    }
}

TEST_CASE("Indexed execution counts ignored prefixes while still choosing the final effective register", "[indexed]") {
    SECTION("Repeated DD prefixes still execute IX displacement stores with an extra prefix cost") {
        CpuHarness h;
        h.cpu.ix.set(0x7204);
        h.load({0xdd, 0xdd, 0x36, 0xfe, 0xa5});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(h.mem[0x7202] == 0xa5);
        REQUIRE(h.cpu.ix.get() == 0x7204);
    }

    SECTION("Mixed DD then FD prefixes leave IY indexed loads in effect") {
        CpuHarness h;
        h.cpu.ix.set(0x7300);
        h.cpu.iy.set(0x7402);
        h.mem[0x7401] = 0x6c;
        h.load({0xdd, 0xfd, 0x7e, 0xff});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.cpu.pc.get() == 0x0004);
        REQUIRE(h.cpu.af.accum() == 0x6c);
        REQUIRE(h.cpu.ix.get() == 0x7300);
        REQUIRE(h.cpu.iy.get() == 0x7402);
    }

    SECTION("An ignored indexed prefix ahead of an ED transfer still consumes bytes and cycles") {
        CpuHarness h;
        h.cpu.de.set(0x0000);
        h.mem.write_addr_to_mem(0x4000, 0xcafe);
        h.load({0xdd, 0xed, 0x5b, 0x00, 0x40});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 24);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(h.cpu.de.get() == 0xcafe);
    }

    SECTION("Repeated FD prefixes keep IY stack exchange semantics with added prefix cost") {
        CpuHarness h;
        h.cpu.iy.set(0x1357);
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x2468);
        h.load({0xfd, 0xfd, 0xe3});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 27);
        REQUIRE(h.cpu.pc.get() == 0x0003);
        REQUIRE(h.cpu.iy.get() == 0x2468);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x1357);
    }
}

TEST_CASE("Block repeat instructions distinguish repeating and terminal iterations", "[extended]") {
    SECTION("LDIR with BC=1 completes in the terminal 16-cycle form without rewinding PC") {
        CpuHarness h;
        h.cpu.hl.set(0x5400);
        h.cpu.de.set(0x6400);
        h.cpu.bc.set(0x0001);
        h.mem[0x5400] = 0x9c;
        h.load({0xed, 0xb0});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.mem[0x6400] == 0x9c);
        REQUIRE(h.cpu.hl.get() == 0x5401);
        REQUIRE(h.cpu.de.get() == 0x6401);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("CPIR exhausts BC without rewinding PC once the final compare completes") {
        CpuHarness h;
        h.cpu.af.accum(0x44);
        h.cpu.hl.set(0x5500);
        h.cpu.bc.set(0x0002);
        h.mem[0x5500] = 0x11;
        h.mem[0x5501] = 0x22;
        h.load({0xed, 0xb1});

        const StepResult first = h.step();
        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.hl.get() == 0x5501);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));

        const StepResult second = h.step();
        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x5502);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
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
        REQUIRE(h.cpu.pc.get() == 0x0001);
    }

    SECTION("CB prefix increments R by two") {
        CpuHarness h;
        h.cpu.bc.hi(0x01);
        h.cpu.ir.lo(0x00);
        h.load({0xcb, 0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x02);
        REQUIRE(h.cpu.pc.get() == 0x0002);
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
        REQUIRE(h.cpu.pc.get() == 0x0004);
    }

    SECTION("R preserves bit 7 while incrementing the low seven bits") {
        CpuHarness h;
        h.cpu.ir.lo(0x80);
        h.load({0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x81);
    }

    SECTION("R wraps the low seven bits after 0x7f") {
        CpuHarness h;
        h.cpu.ir.lo(0x7f);
        h.load({0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x00);
    }

    SECTION("R wraps the low seven bits while preserving bit 7") {
        CpuHarness h;
        h.cpu.ir.lo(0xff);
        h.load({0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x80);
    }

    SECTION("ED prefix increments R by two") {
        CpuHarness h;
        h.cpu.ir.lo(0x10);
        h.cpu.de.set(0x1234);
        h.mem.write_addr_to_mem(0x4000, 0xabcd);
        h.load({0xed, 0x5b, 0x00, 0x40});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x12);
        REQUIRE(h.cpu.de.get() == 0xabcd);
        REQUIRE(h.cpu.pc.get() == 0x0004);
    }

    SECTION("Repeated indexed prefixes increment R for each fetched prefix byte") {
        CpuHarness h;
        h.cpu.ir.lo(0x20);
        h.load({0xdd, 0xfd, 0x00});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x23);
        REQUIRE(h.cpu.pc.get() == 0x0003);
    }

    SECTION("Indexed CB forms with ignored prefixes still count every fetched prefix toward R") {
        CpuHarness h;
        h.cpu.ix.set(0x6200);
        h.cpu.ir.lo(0x30);
        h.mem[0x6200] = 0x01;
        h.load({0xdd, 0xdd, 0xcb, 0x00, 0x46});

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x34);
        REQUIRE(h.cpu.pc.get() == 0x0005);
    }

    SECTION("Maskable interrupts increment R when the interrupt is acknowledged") {
        CpuHarness h;
        h.cpu.ir.lo(0x40);
        h.cpu.iff1 = true;
        h.cpu.iff2 = true;
        h.cpu.interrupt = true;
        h.cpu.int_mode = 1;

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x41);
        REQUIRE(h.cpu.pc.get() == 0x0038);
    }

    SECTION("Mode 2 interrupts increment R when the interrupt is acknowledged") {
        CpuHarness h;
        h.cpu.ir.hi(0x12);
        h.cpu.ir.lo(0x50);
        h.cpu.sp.set(0xfffe);
        h.cpu.iff1 = true;
        h.cpu.iff2 = true;
        h.cpu.interrupt = true;
        h.cpu.int_mode = 2;
        h.mem[0x12ff] = 0x67;
        h.mem[0x1300] = 0x45;

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x51);
        REQUIRE(h.cpu.pc.get() == 0x4567);
    }

    SECTION("NMI increments R before vectoring to 0x0066") {
        CpuHarness h;
        h.cpu.ir.lo(0x60);
        h.cpu.int_nmi = true;

        h.step();
        REQUIRE(h.cpu.ir.lo() == 0x61);
        REQUIRE(h.cpu.pc.get() == 0x0066);
    }
}
