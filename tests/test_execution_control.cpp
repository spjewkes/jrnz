#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Conditional JP executes and skips according to flags", "[control-exec]") {
    SECTION("JP NZ taken") {
        CpuHarness h;
        h.cpu.af.flag(RegisterAF::Flags::Zero, false);
        h.load({0xc2, 0x34, 0x12});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 10);
        REQUIRE(h.cpu.pc.get() == 0x1234);
    }

    SECTION("JP NZ not taken") {
        CpuHarness h;
        h.cpu.af.flag(RegisterAF::Flags::Zero, true);
        h.load({0xc2, 0x34, 0x12});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 10);
        REQUIRE(h.cpu.pc.get() == 0x0003);
    }
}

TEST_CASE("Conditional CALL and RET use documented taken and untaken behavior", "[control-exec]") {
    SECTION("CALL Z taken pushes return address and jumps") {
        CpuHarness h;
        h.cpu.af.flag(RegisterAF::Flags::Zero, true);
        h.load({0xcc, 0x78, 0x56});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 17);
        REQUIRE(h.cpu.pc.get() == 0x5678);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x0003);
    }

    SECTION("CALL Z not taken only advances PC") {
        CpuHarness h;
        h.cpu.af.flag(RegisterAF::Flags::Zero, false);
        h.load({0xcc, 0x78, 0x56});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 10);
        REQUIRE(h.cpu.pc.get() == 0x0003);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
    }

    SECTION("RET C taken pops PC from stack") {
        CpuHarness h;
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x3456);
        h.load({0xd8});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 11);
        REQUIRE(h.cpu.pc.get() == 0x3456);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
    }

    SECTION("RET C not taken leaves stack untouched") {
        CpuHarness h;
        h.cpu.af.flag(RegisterAF::Flags::Carry, false);
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x3456);
        h.load({0xd8});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 5);
        REQUIRE(h.cpu.pc.get() == 0x0001);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
    }
}

TEST_CASE("RST pushes the next PC and jumps to the fixed vector", "[control-exec]") {
    CpuHarness h;
    h.cpu.pc.set(0x0100);
    h.mem[0x0100] = 0xef;  // RST 28h

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 11);
    REQUIRE(h.cpu.pc.get() == 0x0028);
    REQUIRE(h.cpu.sp.get() == 0xfffc);
    REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x0101);
}

TEST_CASE("IM instructions set the interrupt mode operand", "[control-exec]") {
    SECTION("IM 0") {
        CpuHarness h;
        h.load({0xed, 0x46});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.int_mode == 0);
    }

    SECTION("IM 1") {
        CpuHarness h;
        h.load({0xed, 0x56});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.int_mode == 1);
    }

    SECTION("IM 2") {
        CpuHarness h;
        h.load({0xed, 0x5e});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.int_mode == 2);
    }
}

TEST_CASE("LD A,I and LD A,R apply documented flag side effects", "[ir-transfer]") {
    SECTION("LD A,I copies I and mirrors IFF2 into PV while preserving carry") {
        CpuHarness h;
        h.cpu.ir.hi(0x80);
        h.cpu.iff2 = true;
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.af.flag(RegisterAF::Flags::AddSubtract, true);
        h.cpu.af.flag(RegisterAF::Flags::HalfCarry, true);
        h.load({0xed, 0x57});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(h.cpu.af.accum() == 0x80);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }

    SECTION("LD A,R copies the incremented R value and mirrors IFF2 into PV") {
        CpuHarness h;
        h.cpu.ir.lo(0x20);
        h.cpu.iff2 = false;
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x5f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(h.cpu.af.accum() == 0x22);
        REQUIRE(h.cpu.ir.lo() == 0x22);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    }
}

TEST_CASE("RETN restores IFF1 from IFF2 and returns to the stacked address", "[interrupts]") {
    CpuHarness h;
    h.cpu.iff1 = false;
    h.cpu.iff2 = true;
    h.cpu.sp.set(0xfffc);
    h.mem.write_addr_to_mem(0xfffc, 0x4000);
    h.load({0xed, 0x45});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 14);
    REQUIRE(h.cpu.pc.get() == 0x4000);
    REQUIRE(h.cpu.sp.get() == 0xfffe);
    REQUIRE(h.cpu.iff1);
    REQUIRE(h.cpu.iff2);
}

TEST_CASE("RETI returns to the stacked address", "[interrupts]") {
    CpuHarness h;
    h.cpu.iff1 = false;
    h.cpu.iff2 = true;
    h.cpu.sp.set(0xfffc);
    h.mem.write_addr_to_mem(0xfffc, 0x4567);
    h.load({0xed, 0x4d});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 14);
    REQUIRE(h.cpu.pc.get() == 0x4567);
    REQUIRE(h.cpu.sp.get() == 0xfffe);
}

TEST_CASE("Mode 2 interrupt pushes PC and vectors through the I register page", "[interrupts]") {
    CpuHarness h;
    h.cpu.pc.set(0x2000);
    h.cpu.sp.set(0xfffe);
    h.cpu.iff1 = true;
    h.cpu.int_mode = 2;
    h.cpu.ir.hi(0x12);
    h.mem[0x12ff] = 0x56;
    h.mem[0x1300] = 0x34;
    h.cpu.interrupt = true;

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 13);
    REQUIRE(h.cpu.pc.get() == 0x3456);
    REQUIRE(h.cpu.sp.get() == 0xfffc);
    REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x2000);
    REQUIRE_FALSE(h.cpu.interrupt);
}

TEST_CASE("HALT holds PC until an interrupt resumes execution", "[interrupts]") {
    CpuHarness h;
    h.load({0x76});

    const StepResult halt = h.step();
    REQUIRE(halt.cycle_delta() == 4);
    REQUIRE(h.cpu.halted);
    REQUIRE(h.cpu.pc.get() == 0x0001);

    const StepResult stalled = h.step();
    REQUIRE(stalled.cycle_delta() == 4);
    REQUIRE(h.cpu.halted);
    REQUIRE(h.cpu.pc.get() == 0x0001);

    h.cpu.iff1 = true;
    h.cpu.int_mode = 1;
    h.cpu.interrupt = true;

    const StepResult interrupt = h.step();
    REQUIRE(interrupt.cycle_delta() == 13);
    REQUIRE_FALSE(h.cpu.halted);
    REQUIRE(h.cpu.pc.get() == 0x0038);
    REQUIRE(h.cpu.sp.get() == 0xfffc);
    REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x0001);
    REQUIRE_FALSE(h.cpu.interrupt);
}

TEST_CASE("NMI pushes PC, jumps to 0x66, and preserves IFF2 from IFF1", "[interrupts]") {
    CpuHarness h;
    h.cpu.pc.set(0x2345);
    h.cpu.sp.set(0xfffe);
    h.cpu.iff1 = true;
    h.cpu.iff2 = false;
    h.cpu.halted = true;
    h.cpu.int_nmi = true;

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 11);
    REQUIRE(h.cpu.pc.get() == 0x0066);
    REQUIRE(h.cpu.sp.get() == 0xfffc);
    REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x2345);
    REQUIRE_FALSE(h.cpu.iff1);
    REQUIRE(h.cpu.iff2);
    REQUIRE_FALSE(h.cpu.int_nmi);
    REQUIRE_FALSE(h.cpu.halted);
}
