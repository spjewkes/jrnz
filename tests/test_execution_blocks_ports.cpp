#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Decrementing block instructions follow documented transfer and compare behavior", "[blocks-ports]") {
    SECTION("LDD copies one byte and decrements HL DE and BC") {
        CpuHarness h;
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.hl.set(0x8d01);
        h.cpu.de.set(0x8e01);
        h.cpu.bc.set(0x0002);
        h.poke(0x8d01, 0x5c);
        h.load({0xed, 0xa8});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.peek(0x8e01) == 0x5c);
        REQUIRE(h.cpu.hl.get() == 0x8d00);
        REQUIRE(h.cpu.de.get() == 0x8e00);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("LDDR repeats until BC becomes zero while copying backwards") {
        CpuHarness h;
        h.cpu.hl.set(0x8f01);
        h.cpu.de.set(0x9001);
        h.cpu.bc.set(0x0002);
        h.poke(0x8f01, 0xaa);
        h.poke(0x8f00, 0xbb);
        h.load({0xed, 0xb8});

        const StepResult first = h.step();
        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.peek(0x9001) == 0xaa);
        REQUIRE(h.cpu.hl.get() == 0x8f00);
        REQUIRE(h.cpu.de.get() == 0x9000);
        REQUIRE(h.cpu.bc.get() == 0x0001);

        const StepResult second = h.step();
        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.peek(0x9000) == 0xbb);
        REQUIRE(h.cpu.hl.get() == 0x8eff);
        REQUIRE(h.cpu.de.get() == 0x8fff);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("CPI advances until mismatch and CPD decrements on a single step") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.hl.set(0x9100);
        h.cpu.bc.set(0x0002);
        h.poke(0x9100, 0x10);
        h.load({0xed, 0xa1, 0xed, 0xa9});

        const StepResult cpi = h.step();
        REQUIRE(cpi.cycle_delta() == 16);
        REQUIRE(cpi.pc_after == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x9101);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));

        h.cpu.hl.set(0x9101);
        h.cpu.bc.set(0x0002);
        h.poke(0x9101, 0x20);

        const StepResult cpd = h.step();
        REQUIRE(cpd.cycle_delta() == 16);
        REQUIRE(cpd.pc_after == 0x0004);
        REQUIRE(h.cpu.hl.get() == 0x9100);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("CPDR repeats until a match while decrementing HL") {
        CpuHarness h;
        h.cpu.af.accum(0x44);
        h.cpu.hl.set(0x9201);
        h.cpu.bc.set(0x0002);
        h.poke(0x9201, 0x12);
        h.poke(0x9200, 0x44);
        h.load({0xed, 0xb9});

        const StepResult first = h.step();
        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.hl.get() == 0x9200);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));

        const StepResult second = h.step();
        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x91ff);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }

    SECTION("LDDR with BC=1 completes immediately without rewinding PC") {
        CpuHarness h;
        h.cpu.hl.set(0x9300);
        h.cpu.de.set(0x9400);
        h.cpu.bc.set(0x0001);
        h.poke(0x9300, 0x3d);
        h.load({0xed, 0xb8});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.peek(0x9400) == 0x3d);
        REQUIRE(h.cpu.hl.get() == 0x92ff);
        REQUIRE(h.cpu.de.get() == 0x93ff);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("CPDR exhausts BC on the final non-matching compare without rewinding PC") {
        CpuHarness h;
        h.cpu.af.accum(0x7e);
        h.cpu.hl.set(0x9501);
        h.cpu.bc.set(0x0002);
        h.poke(0x9501, 0x10);
        h.poke(0x9500, 0x20);
        h.load({0xed, 0xb9});

        const StepResult first = h.step();
        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.hl.get() == 0x9500);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));

        const StepResult second = h.step();
        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x94ff);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }
}

TEST_CASE("Documented port instructions use the expected ports and flag rules", "[blocks-ports]") {
    SECTION("IN A,(n) reads the immediate port into A") {
        CpuHarness h;
        h.cpu.af.set(0x12c3);
        h.mem.floating_counter = 0;
        h.poke(0x4000, 0x7e);
        h.load({0xdb, 0x01});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 11);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0x7e);
    }

    SECTION("IN A,(n) observes the EAR input bit on port 0xfe") {
        CpuHarness h;
        h.cpu.af.set(0x12c3);
        h.mem.set_input_line(MachineInputLine::Ear, false);
        h.load({0xdb, 0xfe});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 11);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0xbf);
    }

    SECTION("IN B,(C) updates B and the documented flags from the port value") {
        CpuHarness h;
        h.cpu.bc.set(0x00fe);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x40});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.bc.hi() == 0xff);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("IN B,(C) reflects a low EAR input and derives flags from the value read") {
        CpuHarness h;
        h.cpu.bc.set(0x00fe);
        h.mem.set_input_line(MachineInputLine::Ear, false);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x40});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.bc.hi() == 0xbf);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("OUT (n),A writes to the immediate port") {
        CpuHarness h;
        h.cpu.af.accum(0x3f);
        h.cpu.af.flags(0xa5);
        h.load({0xd3, 0xfe});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 11);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.mem.port_254 == 0x3f);
        REQUIRE(h.cpu.af.flags() == 0xa5);
    }

    SECTION("OUT (C),A writes using the BC port address") {
        CpuHarness h;
        h.cpu.af.accum(0x66);
        h.cpu.bc.set(0x12fe);
        h.cpu.af.flags(0x5a);
        h.load({0xed, 0x79});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 13);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.mem.port_254 == 0x66);
        REQUIRE(h.cpu.af.flags() == 0x5a);
    }
}
