#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Block compare instructions derive undocumented flags from A minus value minus HF",
          "[undocumented][flags][compliance][block-cp-matrix]") {
    SECTION("Single-step block compare forms follow the adjusted result rule across more boundary cases") {
        struct CompareCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t hl;
            uint16_t bc;
            uint8_t a;
            uint8_t value;
            uint16_t expected_hl;
            uint16_t expected_bc;
            uint8_t expected_flags;
        };

        const CompareCase cases[] = {
            {"cpi borrow with half-borrow", {0xed, 0xa1}, 0x9400, 0x0002, 0x10, 0x01, 0x9401, 0x0001, 0x3e},
            {"cpi equal keeps adjusted result zero", {0xed, 0xa1}, 0x9402, 0x0002, 0x40, 0x40, 0x9403, 0x0001, 0x46},
            {"cpd signed overflow case", {0xed, 0xa9}, 0x9405, 0x0002, 0x80, 0x01, 0x9404, 0x0001, 0x3e},
            {"cpd borrow from low nibble", {0xed, 0xa9}, 0x9407, 0x0002, 0x00, 0x01, 0x9406, 0x0001, 0xbe},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.af.accum(tc.a);
            h.cpu.af.flags(0x00);
            h.cpu.hl.set(tc.hl);
            h.cpu.bc.set(tc.bc);
            h.mem[tc.hl] = tc.value;
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 16);
            REQUIRE(h.cpu.hl.get() == tc.expected_hl);
            REQUIRE(h.cpu.bc.get() == tc.expected_bc);
            REQUIRE(h.cpu.af.accum() == tc.a);
            require_flags(h.cpu.af.flags(), tc.expected_flags);
        }
    }

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
        h.cpu.af.flags(0x00);
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
        require_f3_f5(h, true, false);
    }

    SECTION("LDD uses the same F3 and F5 rule while decrementing HL and DE") {
        CpuHarness h;
        h.cpu.af.accum(0x01);
        h.cpu.af.flags(0x00);
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
        require_f3_f5(h, true, false);
    }

    SECTION("LDIR applies the same F3 and F5 rule on both repeat and terminal iterations") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x00);
        h.cpu.hl.set(0x8c00);
        h.cpu.de.set(0x8d00);
        h.cpu.bc.set(0x0002);
        h.mem[0x8c00] = 0x08;
        h.mem[0x8c01] = 0x01;
        h.load({0xed, 0xb0});

        const StepResult first = h.step();
        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.mem[0x8d00] == 0x08);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);

        const StepResult second = h.step();
        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.mem[0x8d01] == 0x01);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, false);
    }

    SECTION("LDDR applies the same F3 and F5 rule while copying backwards") {
        CpuHarness h;
        h.cpu.af.accum(0x20);
        h.cpu.af.flags(0x00);
        h.cpu.hl.set(0x8e01);
        h.cpu.de.set(0x8f01);
        h.cpu.bc.set(0x0002);
        h.mem[0x8e01] = 0x08;
        h.mem[0x8e00] = 0x01;
        h.load({0xed, 0xb8});

        const StepResult first = h.step();
        REQUIRE(first.cycle_delta() == 21);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.mem[0x8f01] == 0x08);
        REQUIRE(h.cpu.bc.get() == 0x0001);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, true, false);

        const StepResult second = h.step();
        REQUIRE(second.cycle_delta() == 16);
        REQUIRE(h.cpu.pc.get() == 0x0002);
        REQUIRE(h.mem[0x8f00] == 0x01);
        REQUIRE(h.cpu.bc.get() == 0x0000);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        require_f3_f5(h, false, false);
    }
}

TEST_CASE("Block I/O instructions expose undocumented N and F3/F5 behaviour",
          "[undocumented][flags][compliance][block-io-matrix]") {
    SECTION("Single-step block I/O forms cover more adjusted-sum combinations") {
        struct IoCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t bc;
            uint16_t hl;
            bool preload_port_data;
            uint8_t value;
            uint16_t expected_bc;
            uint16_t expected_hl;
            uint8_t expected_flags;
        };

        const IoCase cases[] = {
            {"ini odd parity case", {0xed, 0xa2}, 0x03ff, 0x9800, true, 0x03, 0x02ff, 0x9801, 0x00},
            {"ind carry and subtract case", {0xed, 0xaa}, 0x02fe, 0x9802, false, 0xff, 0x01fe, 0x9801, 0x17},
            {"outi odd parity case", {0xed, 0xa3}, 0x03fe, 0x9804, false, 0x03, 0x02fe, 0x9805, 0x00},
            {"outd carry and subtract case", {0xed, 0xab}, 0x02fe, 0x9806, false, 0xff, 0x01fe, 0x9805, 0x17},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.bc.set(tc.bc);
            h.cpu.hl.set(tc.hl);
            if (tc.preload_port_data) {
                h.mem[0x4000] = tc.value;
                h.mem.floating_counter = 0;
            } else {
                h.mem[tc.hl] = tc.value;
            }
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 16);
            REQUIRE(h.cpu.bc.get() == tc.expected_bc);
            REQUIRE(h.cpu.hl.get() == tc.expected_hl);
            require_flags(h.cpu.af.flags(), tc.expected_flags);
        }
    }

    SECTION("Repeating block I/O forms preserve their undocumented flag shape on the first iteration") {
        struct RepeatCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t bc;
            uint16_t hl;
            bool preload_port_data;
            uint8_t value;
            uint16_t expected_bc;
            uint16_t expected_hl;
            uint8_t expected_flags;
        };

        const RepeatCase cases[] = {
            {"inir repeat step", {0xed, 0xb2}, 0x02ff, 0x9900, true, 0x01, 0x01ff, 0x9901, 0x04},
            {"indr repeat step", {0xed, 0xba}, 0x02ff, 0x9903, true, 0x01, 0x01ff, 0x9902, 0x04},
            {"otir repeat step", {0xed, 0xb3}, 0x02fe, 0x9904, false, 0x01, 0x01fe, 0x9905, 0x00},
            {"otdr repeat step", {0xed, 0xbb}, 0x02fe, 0x9907, false, 0x01, 0x01fe, 0x9906, 0x04},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.bc.set(tc.bc);
            h.cpu.hl.set(tc.hl);
            if (tc.preload_port_data) {
                h.mem[0x4000] = tc.value;
                h.mem.floating_counter = 0;
            } else {
                h.mem[tc.hl] = tc.value;
            }
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 21);
            REQUIRE(h.cpu.pc.get() == 0x0000);
            REQUIRE(h.cpu.bc.get() == tc.expected_bc);
            REQUIRE(h.cpu.hl.get() == tc.expected_hl);
            require_flags(h.cpu.af.flags(), tc.expected_flags);
        }
    }

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
