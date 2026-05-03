#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Conditional control instructions honor the broader documented condition matrix", "[control-matrix]") {
    SECTION("JR NC takes and skips using the carry flag") {
        CpuHarness h;
        h.load({0x30, 0x02, 0x00, 0x00});

        h.cpu.af.flag(RegisterAF::Flags::Carry, false);
        const StepResult taken = h.step();
        REQUIRE(taken.cycle_delta() == 12);
        REQUIRE(h.cpu.pc.get() == 0x0004);

        h.cpu.pc.set(0x0000);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        const StepResult not_taken = h.step();
        REQUIRE(not_taken.cycle_delta() == 7);
        REQUIRE(h.cpu.pc.get() == 0x0002);
    }

    SECTION("JP PO and JP PE branch according to parity overflow") {
        CpuHarness h;
        h.load({0xe2, 0x34, 0x12, 0xea, 0x78, 0x56});

        h.cpu.af.flag(RegisterAF::Flags::ParityOverflow, false);
        const StepResult jp_po = h.step();
        REQUIRE(jp_po.cycle_delta() == 10);
        REQUIRE(h.cpu.pc.get() == 0x1234);

        h.cpu.pc.set(0x0003);
        h.cpu.af.flag(RegisterAF::Flags::ParityOverflow, true);
        const StepResult jp_pe = h.step();
        REQUIRE(jp_pe.cycle_delta() == 10);
        REQUIRE(h.cpu.pc.get() == 0x5678);
    }

    SECTION("Conditional JP covers zero, carry and sign branches in both directions") {
        struct Case {
            uint8_t opcode;
            uint8_t flags;
            bool taken;
            uint16_t target;
        };

        const Case cases[] = {
            {0xc2, 0x00, true, 0x1234},   // JP NZ
            {0xca, 0x00, false, 0x5678},  // JP Z
            {0xda, 0x01, true, 0x9abc},   // JP C
            {0xf2, 0x80, false, 0xdef0},  // JP P
        };

        for (const auto& test_case : cases) {
            CpuHarness h;
            h.cpu.af.flags(test_case.flags);
            h.load({test_case.opcode, static_cast<uint8_t>(test_case.target & 0x00ff),
                    static_cast<uint8_t>(test_case.target >> 8)});

            const StepResult step = h.step();
            REQUIRE(step.cycle_delta() == 10);
            REQUIRE(h.cpu.pc.get() == (test_case.taken ? test_case.target : 0x0003));
        }
    }

    SECTION("Conditional CALL covers zero carry parity and sign conditions") {
        struct Case {
            uint8_t opcode;
            uint8_t flags;
            bool taken;
            uint16_t target;
        };

        const Case cases[] = {
            {0xc4, 0x00, true, 0x3456},   // CALL NZ
            {0xcc, 0x00, false, 0x4567},  // CALL Z
            {0xdc, 0x01, true, 0x5678},   // CALL C
            {0xec, 0x00, false, 0x6789},  // CALL PE
            {0xfc, 0x80, true, 0x789a},   // CALL M
        };

        for (const auto& test_case : cases) {
            CpuHarness h;
            h.cpu.af.flags(test_case.flags);
            h.cpu.sp.set(0xfffe);
            h.load({test_case.opcode, static_cast<uint8_t>(test_case.target & 0x00ff),
                    static_cast<uint8_t>(test_case.target >> 8)});

            const StepResult step = h.step();
            REQUIRE(step.cycle_delta() == (test_case.taken ? 17 : 10));
            REQUIRE(h.cpu.pc.get() == (test_case.taken ? test_case.target : 0x0003));
            REQUIRE(h.cpu.sp.get() == (test_case.taken ? 0xfffc : 0xfffe));
            if (test_case.taken) {
                REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x0003);
            }
        }
    }

    SECTION("RET P and RET M use the sign flag") {
        CpuHarness h;
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x3456);
        h.load({0xf0, 0xf8});

        h.cpu.af.flag(RegisterAF::Flags::Sign, false);
        const StepResult ret_p = h.step();
        REQUIRE(ret_p.cycle_delta() == 11);
        REQUIRE(h.cpu.pc.get() == 0x3456);
        REQUIRE(h.cpu.sp.get() == 0xfffe);

        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x789a);
        h.cpu.pc.set(0x0001);
        h.cpu.af.flag(RegisterAF::Flags::Sign, true);
        const StepResult ret_m = h.step();
        REQUIRE(ret_m.cycle_delta() == 11);
        REQUIRE(h.cpu.pc.get() == 0x789a);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
    }

    SECTION("Conditional RET covers zero carry and parity with taken and untaken paths") {
        struct Case {
            uint8_t opcode;
            uint8_t flags;
            bool taken;
            uint16_t stacked_pc;
        };

        const Case cases[] = {
            {0xc0, 0x40, false, 0x1111},  // RET NZ
            {0xc8, 0x40, true, 0x2222},   // RET Z
            {0xd0, 0x00, true, 0x3333},   // RET NC
            {0xe8, 0x00, false, 0x4444},  // RET PE
            {0xe0, 0x00, true, 0x5555},   // RET PO
        };

        for (const auto& test_case : cases) {
            CpuHarness h;
            h.cpu.af.flags(test_case.flags);
            h.cpu.sp.set(0xfffc);
            h.mem.write_addr_to_mem(0xfffc, test_case.stacked_pc);
            h.load({test_case.opcode});

            const StepResult step = h.step();
            REQUIRE(step.cycle_delta() == (test_case.taken ? 11 : 5));
            REQUIRE(h.cpu.pc.get() == (test_case.taken ? test_case.stacked_pc : 0x0001));
            REQUIRE(h.cpu.sp.get() == (test_case.taken ? 0xfffe : 0xfffc));
        }
    }

    SECTION("JP (HL) jumps through the register pair without reading immediates") {
        CpuHarness h;
        h.cpu.hl.set(0x4abc);
        h.load({0xe9});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(step.pc_after == 0x4abc);
    }

    SECTION("RST 00h and RST 38h push the correct return addresses") {
        CpuHarness h;
        h.cpu.sp.set(0xfffe);
        h.load({0xc7, 0xff});

        const StepResult rst0 = h.step();
        REQUIRE(rst0.cycle_delta() == 11);
        REQUIRE(h.cpu.pc.get() == 0x0000);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x0001);

        h.cpu.pc.set(0x0001);
        h.cpu.sp.set(0xfffe);
        const StepResult rst38 = h.step();
        REQUIRE(rst38.cycle_delta() == 11);
        REQUIRE(h.cpu.pc.get() == 0x0038);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x0002);
    }
}
