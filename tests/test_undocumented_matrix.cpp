#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Ignored prefix combinations follow last-prefix or no-effect rules",
          "[undocumented][compliance][prefix-matrix]") {
    SECTION("Repeated DD prefixes ignore all but the last prefix") {
        CpuHarness h;
        h.load({0xdd, 0xdd, 0x21, 0x34, 0x12});

        const StepResult step = h.step();

        REQUIRE(h.cpu.ix.get() == 0x1234);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(step.cycle_delta() == 18);
    }

    SECTION("Mixed DD then FD leaves the last prefix in effect") {
        CpuHarness h;
        h.load({0xdd, 0xfd, 0x21, 0x78, 0x56});

        const StepResult step = h.step();

        REQUIRE(h.cpu.iy.get() == 0x5678);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(step.cycle_delta() == 18);
    }

    SECTION("DD before ED is ignored and the ED instruction still executes") {
        CpuHarness h;
        h.cpu.af.accum(0x01);
        h.load({0xdd, 0xed, 0x44});

        const StepResult step = h.step();

        REQUIRE(h.cpu.af.accum() == 0xff);
        REQUIRE(h.cpu.pc.get() == 0x0003);
        REQUIRE(step.cycle_delta() == 12);
    }

    SECTION("FD before ED is ignored and the ED instruction still executes") {
        CpuHarness h;
        h.cpu.ir.hi(0x9a);
        h.cpu.af.flag(RegisterAF::Flags::Carry, true);
        h.load({0xfd, 0xed, 0x57});

        const StepResult step = h.step();

        REQUIRE(h.cpu.af.accum() == 0x9a);
        REQUIRE(h.cpu.pc.get() == 0x0003);
        REQUIRE(step.cycle_delta() == 13);
    }

    SECTION("Repeated FD prefixes ignore all but the last prefix") {
        CpuHarness h;
        h.load({0xfd, 0xfd, 0x21, 0xbc, 0x9a});

        const StepResult step = h.step();

        REQUIRE(h.cpu.iy.get() == 0x9abc);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(step.cycle_delta() == 18);
    }

    SECTION("Mixed FD then DD leaves the last prefix in effect") {
        CpuHarness h;
        h.load({0xfd, 0xdd, 0x21, 0x34, 0x12});

        const StepResult step = h.step();

        REQUIRE(h.cpu.ix.get() == 0x1234);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(step.cycle_delta() == 18);
    }

    SECTION("DD before CB keeps the indexed bit operation in IX mode") {
        CpuHarness h;
        h.cpu.ix.set(0xa300);
        h.cpu.af.accum(0x55);
        h.mem.poke_mapped_for_test(0xa301, 0x81);
        h.load({0xdd, 0xdd, 0xcb, 0x01, 0x07});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 27);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(h.mem[0xa301] == 0x03);
        REQUIRE(h.cpu.af.accum() == 0x03);
    }

    SECTION("FD then DD before CB still leaves the last indexed prefix in effect") {
        CpuHarness h;
        h.cpu.ix.set(0xa340);
        h.cpu.iy.set(0xb000);
        h.cpu.bc.hi(0x12);
        h.mem.poke_mapped_for_test(0xa33f, 0x80);
        h.load({0xfd, 0xdd, 0xcb, 0xff, 0x00});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 27);
        REQUIRE(h.cpu.pc.get() == 0x0005);
        REQUIRE(h.mem[0xa33f] == 0x01);
        REQUIRE(h.cpu.bc.hi() == 0x01);
        REQUIRE(h.cpu.iy.get() == 0xb000);
    }
}

TEST_CASE("Undocumented indexed CB register-copy forms populate every destination register",
          "[undocumented][compliance][indexed-cb-matrix]") {
    SECTION("DDCB rotate forms copy the transformed byte into each register target") {
        struct CopyCase {
            uint8_t opcode;
            uint8_t expected_b;
            uint8_t expected_c;
            uint8_t expected_d;
            uint8_t expected_e;
            uint8_t expected_h;
            uint8_t expected_l;
            uint8_t expected_a;
        };

        const CopyCase cases[] = {
            {0x00, 0x03, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70}, {0x01, 0x10, 0x03, 0x30, 0x40, 0x50, 0x60, 0x70},
            {0x02, 0x10, 0x20, 0x03, 0x40, 0x50, 0x60, 0x70}, {0x03, 0x10, 0x20, 0x30, 0x03, 0x50, 0x60, 0x70},
            {0x04, 0x10, 0x20, 0x30, 0x40, 0x03, 0x60, 0x70}, {0x05, 0x10, 0x20, 0x30, 0x40, 0x50, 0x03, 0x70},
            {0x07, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x03},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO("opcode=0xddcb01" << std::hex << static_cast<unsigned int>(tc.opcode));
            h.cpu.bc.set(0x1020);
            h.cpu.de.set(0x3040);
            h.cpu.hl.set(0x5060);
            h.cpu.af.accum(0x70);
            h.cpu.ix.set(0xa400);
            h.mem.poke_mapped_for_test(0xa401, 0x81);
            h.load({0xdd, 0xcb, 0x01, tc.opcode});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 23);
            REQUIRE(h.cpu.pc.get() == 0x0004);
            REQUIRE(h.mem[0xa401] == 0x03);
            REQUIRE(h.cpu.bc.hi() == tc.expected_b);
            REQUIRE(h.cpu.bc.lo() == tc.expected_c);
            REQUIRE(h.cpu.de.hi() == tc.expected_d);
            REQUIRE(h.cpu.de.lo() == tc.expected_e);
            REQUIRE(h.cpu.hl.hi() == tc.expected_h);
            REQUIRE(h.cpu.hl.lo() == tc.expected_l);
            REQUIRE(h.cpu.af.accum() == tc.expected_a);
        }
    }

    SECTION("FDCB RES forms copy the updated byte into each register target while preserving flags") {
        struct CopyCase {
            uint8_t opcode;
            uint8_t expected_b;
            uint8_t expected_c;
            uint8_t expected_d;
            uint8_t expected_e;
            uint8_t expected_h;
            uint8_t expected_l;
            uint8_t expected_a;
        };

        const CopyCase cases[] = {
            {0x80, 0xfe, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71}, {0x81, 0x11, 0xfe, 0x31, 0x41, 0x51, 0x61, 0x71},
            {0x82, 0x11, 0x21, 0xfe, 0x41, 0x51, 0x61, 0x71}, {0x83, 0x11, 0x21, 0x31, 0xfe, 0x51, 0x61, 0x71},
            {0x84, 0x11, 0x21, 0x31, 0x41, 0xfe, 0x61, 0x71}, {0x85, 0x11, 0x21, 0x31, 0x41, 0x51, 0xfe, 0x71},
            {0x87, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0xfe},
        };

        for (const auto &tc : cases) {
            CpuHarness h;
            INFO("opcode=0xfdcbff" << std::hex << static_cast<unsigned int>(tc.opcode));
            h.cpu.af.flags(0xa5);
            h.cpu.bc.set(0x1121);
            h.cpu.de.set(0x3141);
            h.cpu.hl.set(0x5161);
            h.cpu.af.accum(0x71);
            h.cpu.iy.set(0xa501);
            h.mem.poke_mapped_for_test(0xa500, 0xff);
            h.load({0xfd, 0xcb, 0xff, tc.opcode});

            const StepResult step = h.step();

            REQUIRE(step.cycle_delta() == 23);
            REQUIRE(h.cpu.pc.get() == 0x0004);
            REQUIRE(h.mem[0xa500] == 0xfe);
            REQUIRE(h.cpu.bc.hi() == tc.expected_b);
            REQUIRE(h.cpu.bc.lo() == tc.expected_c);
            REQUIRE(h.cpu.de.hi() == tc.expected_d);
            REQUIRE(h.cpu.de.lo() == tc.expected_e);
            REQUIRE(h.cpu.hl.hi() == tc.expected_h);
            REQUIRE(h.cpu.hl.lo() == tc.expected_l);
            REQUIRE(h.cpu.af.accum() == tc.expected_a);
            REQUIRE(h.cpu.af.flags() == 0xa5);
        }
    }
}
