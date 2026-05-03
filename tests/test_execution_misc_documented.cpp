#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("One-off documented opcodes preserve and update machine state as specified", "[misc-doc]") {
    SECTION("NOP advances PC and R without changing registers or flags") {
        CpuHarness h;
        h.cpu.af.set(0x55aa);
        h.cpu.bc.set(0x1234);
        h.cpu.de.set(0x5678);
        h.cpu.hl.set(0x9abc);
        h.cpu.ir.lo(0x20);
        h.load({0x00});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(step.pc_after == 0x0001);
        REQUIRE(h.cpu.af.get() == 0x55aa);
        REQUIRE(h.cpu.bc.get() == 0x1234);
        REQUIRE(h.cpu.de.get() == 0x5678);
        REQUIRE(h.cpu.hl.get() == 0x9abc);
        REQUIRE(h.cpu.ir.lo() == 0x21);
    }

    SECTION("DI clears both interrupt flip-flops and cancels pending EI") {
        CpuHarness h;
        h.cpu.iff1 = true;
        h.cpu.iff2 = true;
        h.cpu.ei_pending = true;
        h.load({0xf3});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(step.pc_after == 0x0001);
        REQUIRE_FALSE(h.cpu.iff1);
        REQUIRE_FALSE(h.cpu.iff2);
        REQUIRE_FALSE(h.cpu.ei_pending);
    }

    SECTION("LD I,A copies A into I without altering flags") {
        CpuHarness h;
        h.cpu.af.set(0x7ba5);
        h.cpu.ir.hi(0x00);
        h.load({0xed, 0x47});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.ir.hi() == 0x7b);
        REQUIRE(h.cpu.af.get() == 0x7ba5);
    }

    SECTION("LD R,A copies A into R while preserving bit 7 semantics") {
        CpuHarness h;
        h.cpu.af.set(0xc13c);
        h.cpu.ir.lo(0x80);
        h.load({0xed, 0x4f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(step.pc_after == 0x0002);
        REQUIRE(h.cpu.ir.lo() == 0xc1);
        REQUIRE(h.cpu.af.get() == 0xc13c);
    }
}
