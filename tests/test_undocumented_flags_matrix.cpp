#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Block compare instructions derive undocumented flags from A minus value minus HF",
          "[undocumented][flags][compliance][block-cp-matrix]") {
    SECTION("CPI sets F3 from bit 3 and F5 from bit 1 of the adjusted result") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8400);
        h.cpu.bc.set(0x0002);
        h.mem[0x8400] = 0x01;
        h.load({0xed, 0xa1});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.cpu.hl.get() == 0x8401);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, true, false);
    }

    SECTION("CPD uses the same F3 and F5 rule while decrementing HL") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8501);
        h.cpu.bc.set(0x0002);
        h.mem[0x8501] = 0x01;
        h.load({0xed, 0xa9});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.cpu.hl.get() == 0x8500);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);
    }

    SECTION("CPIR keeps the adjusted F3 and F5 values on the repeating step") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8600);
        h.cpu.bc.set(0x0002);
        h.mem[0x8600] = 0x01;
        h.mem[0x8601] = 0x09;
        h.load({0xed, 0xb1});

        const StepResult first = h.step();

        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.hl.get() == 0x8601);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);

        const StepResult second = h.step();

        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x8602);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, false);
    }

    SECTION("CPDR keeps the adjusted F3 and F5 values on the repeating step") {
        CpuHarness h;
        h.cpu.af.accum(0x09);
        h.cpu.hl.set(0x8701);
        h.cpu.bc.set(0x0002);
        h.mem[0x8701] = 0x01;
        h.mem[0x8700] = 0x09;
        h.load({0xed, 0xb9});

        const StepResult first = h.step();

        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.hl.get() == 0x8700);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);

        const StepResult second = h.step();

        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.cpu.hl.get() == 0x86ff);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, false);
    }
}

TEST_CASE("Block transfer instructions derive undocumented flags from A plus transferred byte",
          "[undocumented][flags][compliance][block-ld-matrix]") {
    SECTION("LDI sets F3 and F5 from A plus the copied byte and preserves SZC") {
        CpuHarness h;
        h.cpu.af.accum(0x01);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.cpu.af.flag(RegisterAF::Flags::Sign, true);
        h.cpu.af.flag(RegisterAF::Flags::Zero, true);
        h.cpu.hl.set(0x8800);
        h.cpu.de.set(0x8900);
        h.cpu.bc.set(0x0002);
        h.mem[0x8800] = 0x27;
        h.load({0xed, 0xa0});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem[0x8900] == 0x27);
        REQUIRE(h.cpu.hl.get() == 0x8801);
        REQUIRE(h.cpu.de.get() == 0x8901);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, true);
    }

    SECTION("LDD uses the same F3 and F5 rule while decrementing HL and DE") {
        CpuHarness h;
        h.cpu.af.accum(0x01);
        h.cpu.hl.set(0x8a01);
        h.cpu.de.set(0x8b01);
        h.cpu.bc.set(0x0002);
        h.mem[0x8a01] = 0x27;
        h.load({0xed, 0xa8});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem[0x8b01] == 0x27);
        REQUIRE(h.cpu.hl.get() == 0x8a00);
        REQUIRE(h.cpu.de.get() == 0x8b00);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        require_f3_f5(h, true, true);
    }
}

TEST_CASE("Block I/O instructions expose undocumented N and F3/F5 behaviour",
          "[undocumented][flags][compliance][block-io-matrix]") {
    SECTION("INI takes flags from the input byte, adjusted C, and decremented B") {
        CpuHarness h;
        h.cpu.bc.set(0x02ff);
        h.cpu.hl.set(0x8c00);
        h.mem[0x4000] = 0x01;
        h.mem.floating_counter = 0;
        h.load({0xed, 0xa2});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem[0x8c00] == 0x01);
        REQUIRE(h.cpu.bc.get() == 0x01ff);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("INI sets N H and C when the transferred byte has bit 7 set and the adjusted sum overflows") {
        CpuHarness h;
        h.cpu.bc.set(0x02fe);
        h.cpu.hl.set(0x8c10);
        h.load({0xed, 0xa2});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem[0x8c10] == 0xff);
        REQUIRE(h.cpu.bc.get() == 0x01fe);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("OUTI takes flags from the written byte, new L, and decremented B") {
        CpuHarness h;
        h.cpu.bc.set(0x02fe);
        h.cpu.hl.set(0x8d00);
        h.mem[0x8d00] = 0x01;
        h.load({0xed, 0xa3});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem.port_254 == 0x01);
        REQUIRE(h.cpu.bc.get() == 0x01fe);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("OUTI sets N H and C when the written byte has bit 7 set and the adjusted sum overflows") {
        CpuHarness h;
        h.cpu.bc.set(0x02fe);
        h.cpu.hl.set(0x8e00);
        h.mem[0x8e00] = 0xff;
        h.load({0xed, 0xa3});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem.port_254 == 0xff);
        REQUIRE(h.cpu.bc.get() == 0x01fe);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("IND takes flags from the input byte, adjusted C, and decremented B while moving HL backwards") {
        CpuHarness h;
        h.cpu.bc.set(0x02ff);
        h.cpu.hl.set(0x8f10);
        h.mem[0x4000] = 0x01;
        h.mem.floating_counter = 0;
        h.load({0xed, 0xaa});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem[0x8f10] == 0x01);
        REQUIRE(h.cpu.hl.get() == 0x8f0f);
        REQUIRE(h.cpu.bc.get() == 0x01ff);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("IND sets N H and C when the transferred byte has bit 7 set and the adjusted sum underflows") {
        CpuHarness h;
        h.cpu.bc.set(0x02fe);
        h.cpu.hl.set(0x8f20);
        h.load({0xed, 0xaa});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem[0x8f20] == 0xff);
        REQUIRE(h.cpu.hl.get() == 0x8f1f);
        REQUIRE(h.cpu.bc.get() == 0x01fe);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("INDR keeps the repeating-step undocumented flags while decrementing HL") {
        CpuHarness h;
        h.cpu.bc.set(0x02ff);
        h.cpu.hl.set(0x9031);
        h.mem[0x4000] = 0x01;
        h.mem.floating_counter = 0;
        h.load({0xed, 0xba});

        const StepResult first = h.step();

        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.mem[0x9031] == 0x01);
        REQUIRE(h.cpu.hl.get() == 0x9030);
        REQUIRE(h.cpu.bc.get() == 0x01ff);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("OUTD takes flags from the written byte, new L, and decremented B") {
        CpuHarness h;
        h.cpu.bc.set(0x02fe);
        h.cpu.hl.set(0x9121);
        h.mem[0x9121] = 0x01;
        h.load({0xed, 0xab});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem.port_254 == 0x01);
        REQUIRE(h.cpu.hl.get() == 0x9120);
        REQUIRE(h.cpu.bc.get() == 0x01fe);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("OUTD sets N H and C when the written byte has bit 7 set and the adjusted sum overflows") {
        CpuHarness h;
        h.cpu.bc.set(0x02fe);
        h.cpu.hl.set(0x9201);
        h.mem[0x9201] = 0xff;
        h.load({0xed, 0xab});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 16);
        REQUIRE(h.mem.port_254 == 0xff);
        REQUIRE(h.cpu.hl.get() == 0x9200);
        REQUIRE(h.cpu.bc.get() == 0x01fe);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }

    SECTION("OTDR keeps the repeating-step undocumented flags while decrementing HL") {
        CpuHarness h;
        h.cpu.bc.set(0x02fe);
        h.cpu.hl.set(0x9301);
        h.mem[0x9301] = 0x01;
        h.load({0xed, 0xbb});

        const StepResult first = h.step();

        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.mem.port_254 == 0x01);
        REQUIRE(h.cpu.hl.get() == 0x9300);
        REQUIRE(h.cpu.bc.get() == 0x01fe);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        require_f3_f5(h, false, false);
    }
}
