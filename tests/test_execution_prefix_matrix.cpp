#include <array>
#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Prefix-heavy execution paths honor the last effective prefix and consumed bytes",
          "[indexed][compliance][prefix-exec-matrix]") {
    SECTION("Mixed DD and FD chains select the expected register pair for indexed loads and stores") {
        struct PrefixCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t ix;
            uint16_t iy;
            uint16_t expected_ix;
            uint16_t expected_iy;
            uint16_t expected_pc;
            uint64_t expected_cycles;
            uint16_t expected_mem_addr;
            uint8_t expected_mem_value;
        };

        const PrefixCase cases[] = {
            {"dd fd ld iy,nn",
             {0xdd, 0xfd, 0x21, 0x78, 0x56},
             0x1111,
             0x2222,
             0x1111,
             0x5678,
             0x0005,
             18,
             0x0000,
             0x00},
            {"fd dd ld ix,nn",
             {0xfd, 0xdd, 0x21, 0x34, 0x12},
             0x1111,
             0x2222,
             0x1234,
             0x2222,
             0x0005,
             18,
             0x0000,
             0x00},
            {"dd dd ld (ix+d),n",
             {0xdd, 0xdd, 0x36, 0xfe, 0xa5},
             0x7204,
             0x0000,
             0x7204,
             0x0000,
             0x0005,
             23,
             0x7202,
             0xa5},
            {"fd fd ld (iy+d),n",
             {0xfd, 0xfd, 0x36, 0x02, 0x5a},
             0x0000,
             0x7300,
             0x0000,
             0x7300,
             0x0005,
             23,
             0x7302,
             0x5a},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.ix.set(tc.ix);
            h.cpu.iy.set(tc.iy);
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == tc.expected_cycles);
            REQUIRE(h.cpu.pc.get() == tc.expected_pc);
            REQUIRE(h.cpu.ix.get() == tc.expected_ix);
            REQUIRE(h.cpu.iy.get() == tc.expected_iy);
            if (tc.expected_mem_addr != 0x0000) {
                REQUIRE(h.mem[tc.expected_mem_addr] == tc.expected_mem_value);
            }
        }
    }

    SECTION("Ignored indexed prefixes ahead of ED opcodes still consume prefix cycles while keeping ED semantics") {
        struct EdCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t de;
            uint16_t sp;
            uint16_t expected_de;
            uint16_t expected_sp;
            uint16_t expected_pc;
            uint64_t expected_cycles;
        };

        const EdCase cases[] = {
            {"dd before ld de,(nn)", {0xdd, 0xed, 0x5b, 0x00, 0x40}, 0x0000, 0xfffe, 0xcafe, 0xfffe, 0x0005, 24},
            {"fd before ld sp,(nn)", {0xfd, 0xed, 0x7b, 0x10, 0x40}, 0x0000, 0x0000, 0x0000, 0xbeef, 0x0005, 24},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.de.set(tc.de);
            h.cpu.sp.set(tc.sp);
            h.mem.write_addr_to_mem(0x4000, 0xcafe);
            h.mem.write_addr_to_mem(0x4010, 0xbeef);
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == tc.expected_cycles);
            REQUIRE(h.cpu.pc.get() == tc.expected_pc);
            REQUIRE(h.cpu.de.get() == tc.expected_de);
            REQUIRE(h.cpu.sp.get() == tc.expected_sp);
        }
    }

    SECTION("Prefix-heavy indexed CB forms honor signed displacements and destination registers") {
        struct IndexedCbCase {
            const char *name;
            std::array<uint8_t, 5> code;
            uint16_t ix;
            uint16_t iy;
            uint16_t addr;
            uint8_t initial;
            uint8_t expected_mem;
            uint8_t expected_b;
            uint8_t expected_a;
        };

        const IndexedCbCase cases[] = {
            {"dd dd cb positive displacement",
             {0xdd, 0xdd, 0xcb, 0x01, 0x00},
             0xa400,
             0x0000,
             0xa401,
             0x81,
             0x03,
             0x03,
             0x70},
            {"fd dd cb negative displacement uses ix",
             {0xfd, 0xdd, 0xcb, 0xff, 0x00},
             0xa500,
             0xb000,
             0xa4ff,
             0x80,
             0x01,
             0x01,
             0x70},
            {"dd fd cb negative displacement uses iy",
             {0xdd, 0xfd, 0xcb, 0xff, 0x00},
             0xa500,
             0xb100,
             0xb0ff,
             0x80,
             0x01,
             0x01,
             0x70},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.bc.set(0x1020);
            h.cpu.af.accum(0x70);
            h.cpu.ix.set(tc.ix);
            h.cpu.iy.set(tc.iy);
            h.mem[tc.addr] = tc.initial;
            h.load({tc.code[0], tc.code[1], tc.code[2], tc.code[3], tc.code[4]});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 27);
            REQUIRE(h.cpu.pc.get() == 0x0005);
            REQUIRE(h.mem[tc.addr] == tc.expected_mem);
            REQUIRE(h.cpu.bc.hi() == tc.expected_b);
            REQUIRE(h.cpu.af.accum() == tc.expected_a);
        }
    }
}
