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
        require_f3_f5(h, true, true);
    }

    SECTION("Odd port reads the floating bus and still only updates flags") {
        CpuHarness h;
        h.cpu.af.set(0xaa01);
        h.cpu.bc.set(0x01ff);
        h.cpu.de.set(0x1234);
        h.cpu.hl.set(0x5678);
        h.poke(0x4000, 0x00);
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
        require_f3_f5(h, false, false);
    }
}

TEST_CASE("CP follows undocumented F3 and F5 operand bits", "[undocumented][flags][cp]") {
    CpuHarness h;
    h.cpu.af.accum(0x30);
    h.load({0xfe, 0x08});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 7);
    REQUIRE(h.cpu.af.accum() == 0x30);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    require_f3_f5(h, true, false);
}

TEST_CASE("BIT copies undocumented flag bits from the tested source", "[undocumented][flags][bit]") {
    SECTION("Register BIT copies F3 and F5 from the tested value") {
        CpuHarness h;
        h.cpu.bc.hi(0x28);
        h.load({0xcb, 0x58});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, true, true);
    }

    SECTION("Indexed BIT copies F3 and F5 from the effective address high byte") {
        CpuHarness h;
        h.cpu.ix.set(0x2834);
        h.poke(0x2834, 0x00);
        h.load({0xdd, 0xcb, 0x00, 0x46});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 20);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, true);
    }

    SECTION("BIT (HL) copies F3 and F5 from MEMPTR rather than the tested value") {
        CpuHarness h;
        h.cpu.hl.set(0x4000);
        h.cpu.memptr.set(0x2801);
        h.poke(0x4000, 0x00);
        h.load({0xcb, 0x46});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 12);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, true);
    }

    SECTION("BIT (HL) observes MEMPTR seeded by LD SP,(nn)") {
        CpuHarness h;
        h.cpu.hl.set(0x4000);
        h.poke(0x2000, 0x00);
        h.poke(0x2001, 0x40);
        h.poke(0x4000, 0x00);
        h.load({0xed, 0x7b, 0x00, 0x20, 0xcb, 0x46});

        const StepResult load_sp = h.step();
        const StepResult bit_hl = h.step();

        REQUIRE(load_sp.cycle_delta() == 20);
        REQUIRE(h.cpu.sp.get() == 0x4000);
        REQUIRE(bit_hl.cycle_delta() == 12);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, true);
    }
}

TEST_CASE("MEMPTR seed instructions are visible through a following BIT (HL) probe", "[undocumented][flags][memptr]") {
    auto require_probe = [](CpuHarness &h, uint16_t expected_memptr) {
        REQUIRE(h.cpu.memptr.get() == expected_memptr);
        const StepResult bit_hl = h.step();
        REQUIRE(bit_hl.cycle_delta() == 12);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, (expected_memptr & 0x0800) != 0, (expected_memptr & 0x2000) != 0);
    };

    SECTION("LD A,(BC) seeds MEMPTR with BC+1") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.bc.set(0x2800);
        h.poke(0x2800, 0x12);
        h.poke(0x5000, 0x00);
        h.load({0x0a, 0xcb, 0x46});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 7);
        REQUIRE(h.cpu.af.accum() == 0x12);
        require_probe(h, 0x2801);
    }

    SECTION("LD (DE),A seeds MEMPTR with A in the high byte and DE+1 low byte") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.af.accum(0x28);
        h.cpu.de.set(0x8401);
        h.poke(0x5000, 0x00);
        h.load({0x12, 0xcb, 0x46});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 7);
        REQUIRE(h.peek(0x8401) == 0x28);
        require_probe(h, 0x2802);
    }

    SECTION("LD A,(nn) seeds MEMPTR with nn+1") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.poke(0x3456, 0x99);
        h.poke(0x5000, 0x00);
        h.load({0x3a, 0x56, 0x34, 0xcb, 0x46});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 13);
        REQUIRE(h.cpu.af.accum() == 0x99);
        require_probe(h, 0x3457);
    }

    SECTION("LD (nn),A seeds MEMPTR with A in the high byte and nn+1 low byte") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.af.accum(0x20);
        h.poke(0x5000, 0x00);
        h.load({0x32, 0x56, 0x84, 0xcb, 0x46});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 13);
        REQUIRE(h.peek(0x8456) == 0x20);
        require_probe(h, 0x2057);
    }

    SECTION("JP nn seeds MEMPTR with the jump target even before BIT (HL)") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.poke(0x5000, 0x00);
        h.poke(0x2860, 0xcb);
        h.poke(0x2861, 0x46);
        h.load({0xc3, 0x60, 0x28});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 10);
        REQUIRE(h.cpu.pc.get() == 0x2860);
        require_probe(h, 0x2860);
    }

    SECTION("Taken JR seeds MEMPTR with the branch destination") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.poke(0x5000, 0x00);
        h.poke(0x0004, 0xcb);
        h.poke(0x0005, 0x46);
        h.load({0x18, 0x02});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 12);
        REQUIRE(h.cpu.pc.get() == 0x0004);
        require_probe(h, 0x0004);
    }

    SECTION("Taken DJNZ seeds MEMPTR with the branch destination") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.bc.hi(0x02);
        h.poke(0x5000, 0x00);
        h.poke(0x0004, 0xcb);
        h.poke(0x0005, 0x46);
        h.load({0x10, 0x02});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 13);
        REQUIRE(h.cpu.pc.get() == 0x0004);
        REQUIRE(h.cpu.bc.hi() == 0x01);
        require_probe(h, 0x0004);
    }

    SECTION("CALL nn seeds MEMPTR with the call target") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.sp.set(0xfffe);
        h.poke(0x5000, 0x00);
        h.poke(0x2860, 0xcb);
        h.poke(0x2861, 0x46);
        h.load({0xcd, 0x60, 0x28});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 17);
        REQUIRE(h.cpu.pc.get() == 0x2860);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x0003);
        require_probe(h, 0x2860);
    }

    SECTION("RET seeds MEMPTR with the popped return address") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x2860);
        h.poke(0x5000, 0x00);
        h.poke(0x2860, 0xcb);
        h.poke(0x2861, 0x46);
        h.load({0xc9});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 10);
        REQUIRE(h.cpu.pc.get() == 0x2860);
        require_probe(h, 0x2860);
    }

    SECTION("Indexed memory loads seed MEMPTR with the effective address") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.ix.set(0x2804);
        h.poke(0x2802, 0x44);
        h.poke(0x5000, 0x00);
        h.load({0xdd, 0x7e, 0xfe, 0xcb, 0x46});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 19);
        REQUIRE(h.cpu.af.accum() == 0x44);
        require_probe(h, 0x2802);
    }

    SECTION("Indexed memory stores seed MEMPTR with the effective address") {
        CpuHarness h;
        h.cpu.hl.set(0x5000);
        h.cpu.iy.set(0xa003);
        h.cpu.af.accum(0x77);
        h.poke(0x5000, 0x00);
        h.load({0xfd, 0x77, 0xff, 0xcb, 0x46});

        const StepResult seed = h.step();
        REQUIRE(seed.cycle_delta() == 19);
        REQUIRE(h.peek(0xa002) == 0x77);
        require_probe(h, 0xa002);
    }
}

TEST_CASE("LD A,I and LD A,R copy undocumented flag bits from the loaded value", "[undocumented][flags][ir]") {
    SECTION("LD A,I copies F3 and F5 from I") {
        CpuHarness h;
        h.cpu.ir.hi(0x28);
        h.load({0xed, 0x57});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(h.cpu.af.accum() == 0x28);
        require_f3_f5(h, true, true);
    }

    SECTION("LD A,R copies F3 and F5 from the incremented R value") {
        CpuHarness h;
        h.cpu.ir.lo(0x26);
        h.load({0xed, 0x5f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 9);
        REQUIRE(h.cpu.af.accum() == 0x28);
        require_f3_f5(h, true, true);
    }
}

TEST_CASE("SCF CCF and CPL copy undocumented flag bits from A or the result", "[undocumented][flags][misc]") {
    SECTION("SCF copies F3 and F5 from A") {
        CpuHarness h;
        h.cpu.af.accum(0x28);
        h.cpu.af.flag(RegisterAF::Flags::AddSubtract, true);
        h.cpu.af.flag(RegisterAF::Flags::HalfCarry, true);
        h.cpu.flags_modified_last_instruction = true;
        h.load({0x37});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("SCF ORs in prior F3 and F5 when the previous instruction did not modify flags") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x08);
        h.load({0x40, 0x37});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("SCF takes F3 and F5 directly from A when the previous instruction modified flags") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x08);
        h.load({0xb7, 0x37});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, false, true);
    }

    SECTION("CCF copies F3 and F5 from A while half-carry follows old carry") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x01);
        h.cpu.flags_modified_last_instruction = true;
        h.load({0x3f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }

    SECTION("CCF ORs in prior F3 and F5 when the previous instruction did not modify flags") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x20);
        h.load({0x40, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("CCF takes F3 and F5 directly from A when the previous instruction modified flags") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x20);
        h.load({0xb7, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }

    SECTION("CPL copies F3 and F5 from the complemented result") {
        CpuHarness h;
        h.cpu.af.accum(0xd7);
        h.load({0x2f});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x28);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("SCF followed by CCF uses direct A bits on the second step because SCF modified flags") {
        CpuHarness h;
        h.cpu.af.accum(0x28);
        h.cpu.af.flags(0x00);
        h.load({0x37, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("CCF followed by SCF uses direct A bits on the second step because CCF modified flags") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x01);
        h.load({0x3f, 0x37});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }
}

TEST_CASE("Rotate and shift families copy undocumented F3 and F5 bits from the result",
          "[undocumented][flags][rotate-shift]") {
    SECTION("CB-prefixed rotate copies F3 and F5 from the transformed byte") {
        CpuHarness h;
        h.cpu.bc.hi(0x14);
        h.load({0xcb, 0x00});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x28);
        require_f3_f5(h, true, true);
    }

    SECTION("Accumulator rotate copies F3 and F5 from the new A value while preserving SZPV") {
        CpuHarness h;
        h.cpu.af.accum(0x84);
        h.cpu.af.flags(0xc4);
        h.load({0x07});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x09);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);
    }

    SECTION("Shift copies F3 and F5 from the shifted result") {
        CpuHarness h;
        h.cpu.bc.hi(0x48);
        h.load({0xcb, 0x20});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x90);
        require_f3_f5(h, false, false);
    }
}

TEST_CASE("CCF after selected instructions uses the authentic previous-instruction latch behavior",
          "[undocumented][flags][ccf-after]") {
    SECTION("BIT (HL) then CCF takes F3 and F5 from A rather than BIT's MEMPTR-derived flags") {
        CpuHarness h;
        h.cpu.af.accum(0x28);
        h.cpu.af.flags(0x01);
        h.cpu.hl.set(0x8800);
        h.cpu.memptr.set(0x1200);
        h.poke(0x8800, 0x01);
        h.load({0xcb, 0x46, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 12);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("BIT (IX+d) then CCF takes F3 and F5 from A rather than the indexed address high byte") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x01);
        h.cpu.ix.set(0x2401);
        h.poke(0x2400, 0x01);
        h.load({0xdd, 0xcb, 0xff, 0x46, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 20);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }

    SECTION("LD A,I then CCF takes F3 and F5 from the loaded A value") {
        CpuHarness h;
        h.cpu.ir.hi(0x28);
        h.cpu.iff2 = true;
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x57, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 9);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x28);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("LD A,R then CCF takes F3 and F5 from the incremented R value now in A") {
        CpuHarness h;
        h.cpu.ir.lo(0x26);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xed, 0x5f, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 9);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x28);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, true);
    }

    SECTION("LDI then CCF uses direct A bits after the block-transfer flag update") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x01);
        h.cpu.hl.set(0x8a00);
        h.cpu.de.set(0x8b00);
        h.cpu.bc.set(0x0001);
        h.poke(0x8a00, 0x08);
        h.load({0xed, 0xa0, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 16);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.peek(0x8b00) == 0x08);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, false, true);
    }

    SECTION("CPIR then CCF uses direct A bits after the block-compare flag update") {
        CpuHarness h;
        h.cpu.af.accum(0x08);
        h.cpu.af.flags(0x01);
        h.cpu.hl.set(0x8c00);
        h.cpu.bc.set(0x0001);
        h.poke(0x8c00, 0x07);
        h.load({0xed, 0xb1, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 16);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }

    SECTION("OUTI then CCF uses direct A bits after block I/O updated flags") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x01);
        h.cpu.bc.set(0x01fe);
        h.cpu.hl.set(0x8d00);
        h.poke(0x8d00, 0x08);
        h.load({0xed, 0xa3, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 16);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, false, true);
    }

    SECTION("RLCA then CCF uses direct A bits after the rotate updated flags") {
        CpuHarness h;
        h.cpu.af.accum(0x84);
        h.cpu.af.flags(0x01);
        h.load({0x07, 0x3f});

        const StepResult first = h.step();
        const StepResult second = h.step();

        REQUIRE(first.cycle_delta() == 4);
        REQUIRE(second.cycle_delta() == 4);
        REQUIRE(h.cpu.af.accum() == 0x09);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        require_f3_f5(h, true, false);
    }
}
