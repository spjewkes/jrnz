#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Interrupt state transitions follow the current compliance matrix",
          "[interrupts][compliance][interrupt-matrix]") {
    SECTION("Interrupt and NMI entry paths update control state consistently") {
        struct EntryCase {
            const char *name;
            bool halted;
            bool iff1;
            bool iff2;
            bool interrupt;
            bool int_nmi;
            uint8_t int_mode;
            uint16_t pc;
            uint16_t sp;
            uint8_t r_hi;
            uint8_t r_lo;
            uint16_t vector_addr;
            uint16_t vector_target;
            uint16_t expected_pc;
            uint16_t expected_sp;
            uint16_t expected_stack;
            uint8_t expected_r_lo;
            bool expected_halted;
            bool expected_iff1;
            bool expected_iff2;
            bool expected_interrupt;
            bool expected_int_nmi;
            uint64_t expected_cycles;
        };

        const EntryCase cases[] = {
            {"im1 interrupt", false,  true,   true,   true, false, 1,     0x2000, 0xfffe, 0x00,  0x20, 0x0000,
             0x0000,          0x0038, 0xfffc, 0x2000, 0x21, false, false, false,  false,  false, 13},
            {"im0 interrupt uses mode 1 vector",
             false,
             true,
             true,
             true,
             false,
             0,
             0x2345,
             0xfffe,
             0x00,
             0x30,
             0x0000,
             0x0000,
             0x0038,
             0xfffc,
             0x2345,
             0x31,
             false,
             false,
             false,
             false,
             false,
             13},
            {"im2 interrupt vectors through I page",
             false,
             true,
             false,
             true,
             false,
             2,
             0x3456,
             0xfffe,
             0x9a,
             0x40,
             0x9aff,
             0x5678,
             0x5678,
             0xfffc,
             0x3456,
             0x41,
             false,
             false,
             false,
             false,
             false,
             13},
            {"nmi releases halt and preserves old iff1 into iff2",
             true,
             true,
             false,
             false,
             true,
             1,
             0x4567,
             0xfffe,
             0x00,
             0x50,
             0x0000,
             0x0000,
             0x0066,
             0xfffc,
             0x4567,
             0x51,
             false,
             false,
             true,
             false,
             false,
             11},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.halted = tc.halted;
            h.cpu.iff1 = tc.iff1;
            h.cpu.iff2 = tc.iff2;
            h.cpu.interrupt = tc.interrupt;
            h.cpu.int_nmi = tc.int_nmi;
            h.cpu.int_mode = tc.int_mode;
            h.cpu.pc.set(tc.pc);
            h.cpu.sp.set(tc.sp);
            h.cpu.ir.hi(tc.r_hi);
            h.cpu.ir.lo(tc.r_lo);
            if (tc.int_mode == 2 && tc.vector_addr != 0x0000) {
                h.poke(tc.vector_addr, static_cast<uint8_t>(tc.vector_target & 0xff));
                h.poke(tc.vector_addr + 1, static_cast<uint8_t>((tc.vector_target >> 8) & 0xff));
            }

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == tc.expected_cycles);
            REQUIRE(h.cpu.pc.get() == tc.expected_pc);
            REQUIRE(h.cpu.sp.get() == tc.expected_sp);
            REQUIRE(h.mem.read_addr_from_mem(tc.expected_sp) == tc.expected_stack);
            REQUIRE(h.cpu.ir.lo() == tc.expected_r_lo);
            REQUIRE(h.cpu.halted == tc.expected_halted);
            REQUIRE(h.cpu.iff1 == tc.expected_iff1);
            REQUIRE(h.cpu.iff2 == tc.expected_iff2);
            REQUIRE(h.cpu.interrupt == tc.expected_interrupt);
            REQUIRE(h.cpu.int_nmi == tc.expected_int_nmi);
        }
    }

    SECTION("EI and DI transition combinations update pending and visible interrupt state") {
        struct PendingCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            bool interrupt;
            bool iff1;
            bool iff2;
            bool ei_pending;
            uint16_t expected_pc_after_first;
            bool expected_iff1_after_first;
            bool expected_iff2_after_first;
            bool expected_pending_after_first;
            uint16_t expected_pc_after_second;
            bool expected_iff1_after_second;
            bool expected_iff2_after_second;
            bool expected_pending_after_second;
        };

        const PendingCase cases[] = {
            {"ei then nop enables after the following instruction",
             {0xfb, 0x00},
             true,
             false,
             false,
             false,
             0x0001,
             false,
             false,
             true,
             0x0002,
             true,
             true,
             false},
            {"ei then di cancels the pending enable",
             {0xfb, 0xf3},
             true,
             false,
             false,
             false,
             0x0001,
             false,
             false,
             true,
             0x0002,
             false,
             false,
             false},
            {"di leaves both flip-flops cleared immediately",
             {0xf3, 0x00},
             true,
             true,
             true,
             false,
             0x0038,
             false,
             false,
             false,
             0x0039,
             false,
             false,
             false},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.interrupt = tc.interrupt;
            h.cpu.iff1 = tc.iff1;
            h.cpu.iff2 = tc.iff2;
            h.cpu.ei_pending = tc.ei_pending;
            h.load(tc.code);

            const StepResult first = h.step();
            REQUIRE(first.pc_after == tc.expected_pc_after_first);
            REQUIRE(h.cpu.iff1 == tc.expected_iff1_after_first);
            REQUIRE(h.cpu.iff2 == tc.expected_iff2_after_first);
            REQUIRE(h.cpu.ei_pending == tc.expected_pending_after_first);

            const StepResult second = h.step();
            REQUIRE(second.pc_after == tc.expected_pc_after_second);
            REQUIRE(h.cpu.iff1 == tc.expected_iff1_after_second);
            REQUIRE(h.cpu.iff2 == tc.expected_iff2_after_second);
            REQUIRE(h.cpu.ei_pending == tc.expected_pending_after_second);
        }
    }
}
