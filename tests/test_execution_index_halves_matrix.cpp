#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Indexed half-register arithmetic and compare edge cases match 8-bit flag rules",
          "[index-halves][compliance][index-halves-matrix]") {
    SECTION("IX half-register arithmetic covers half-carry carry and overflow boundaries") {
        struct ArithmeticCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t ix;
            uint8_t a;
            uint8_t initial_flags;
            uint8_t result;
            uint8_t flags;
        };

        const ArithmeticCase cases[] = {
            {"add a,ixl half-carry", {0xdd, 0x85}, 0x0001, 0x0f, 0x00, 0x10, 0x10},
            {"adc a,ixh carry-in to zero", {0xdd, 0x8c}, 0x0000, 0xff, 0x01, 0x00, 0x51},
            {"sub a,ixh half-borrow", {0xdd, 0x94}, 0x0100, 0x10, 0x00, 0x0f, 0x1a},
            {"sbc a,ixl signed overflow", {0xdd, 0x9d}, 0x0000, 0x80, 0x01, 0x7f, 0x3e},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.ix.set(tc.ix);
            h.cpu.af.accum(tc.a);
            h.cpu.af.flags(tc.initial_flags);
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 8);
            REQUIRE(step.pc_after == 0x0002);
            REQUIRE(h.cpu.af.accum() == tc.result);
            require_flags(h.cpu.af.flags(), tc.flags);
        }
    }

    SECTION("IY half-register compare forms preserve A and expose undocumented result bits") {
        struct CompareCase {
            const char *name;
            std::initializer_list<uint8_t> code;
            uint16_t iy;
            uint8_t a;
            uint8_t flags;
        };

        const CompareCase cases[] = {
            {"cp iyh borrow", {0xfd, 0xbc}, 0x0100, 0x00, 0xbb},
            {"cp iyl equal", {0xfd, 0xbd}, 0x0040, 0x40, 0x42},
            {"cp iyl signed overflow", {0xfd, 0xbd}, 0x00ff, 0x7f, 0x87},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO(tc.name);
            h.cpu.iy.set(tc.iy);
            h.cpu.af.accum(tc.a);
            h.cpu.af.flags(0x00);
            h.load(tc.code);

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 8);
            REQUIRE(step.pc_after == 0x0002);
            REQUIRE(h.cpu.af.accum() == tc.a);
            require_flags(h.cpu.af.flags(), tc.flags);
        }
    }
}
