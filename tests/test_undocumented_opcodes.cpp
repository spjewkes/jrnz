#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Undocumented SLL register and memory forms behave consistently", "[undocumented][sll]") {
    SECTION("SLL B shifts left and forces bit 0 to one") {
        CpuHarness h;
        h.cpu.bc.hi(0x80);
        h.load({0xcb, 0x30});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.bc.hi() == 0x01);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    }

    SECTION("SLL (HL) updates memory and flags") {
        CpuHarness h;
        h.cpu.hl.set(0xa000);
        h.mem[0xa000] = 0x40;
        h.load({0xcb, 0x36});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 15);
        REQUIRE(h.mem[0xa000] == 0x81);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    }

    SECTION("SLL (IX+d) uses indexed memory") {
        CpuHarness h;
        h.cpu.ix.set(0xa100);
        h.mem[0xa102] = 0x80;
        h.load({0xdd, 0xcb, 0x02, 0x36});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.mem[0xa102] == 0x01);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    }

    SECTION("SLL (IY+d) uses indexed memory") {
        CpuHarness h;
        h.cpu.iy.set(0xa200);
        h.mem[0xa1ff] = 0x01;
        h.load({0xfd, 0xcb, 0xff, 0x36});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.mem[0xa1ff] == 0x03);
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    }
}

TEST_CASE("Undocumented NEG aliases match the documented NEG behavior", "[undocumented][neg-alias]") {
    const uint8_t aliases[] = {0x4c, 0x54, 0x5c, 0x64, 0x6c, 0x74, 0x7c};

    for (uint8_t alias : aliases) {
        CpuHarness h;
        h.cpu.af.accum(0x80);
        h.load({0xed, alias});

        const StepResult step = h.step();

        INFO("opcode=0xed" << std::hex << static_cast<uint32_t>(alias));
        REQUIRE(step.cycle_delta() == 8);
        REQUIRE(h.cpu.af.accum() == 0x80);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Sign));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }
}

TEST_CASE("Undocumented RETN aliases restore IFF1 and return from the stack", "[undocumented][retn-alias]") {
    const uint8_t aliases[] = {0x55, 0x5d, 0x65, 0x6d, 0x75, 0x7d};

    for (uint8_t alias : aliases) {
        CpuHarness h;
        h.cpu.iff1 = false;
        h.cpu.iff2 = true;
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x6789);
        h.load({0xed, alias});

        const StepResult step = h.step();

        INFO("opcode=0xed" << std::hex << static_cast<uint32_t>(alias));
        REQUIRE(step.cycle_delta() == 14);
        REQUIRE(h.cpu.pc.get() == 0x6789);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
        REQUIRE(h.cpu.iff1);
        REQUIRE(h.cpu.iff2);
    }
}

TEST_CASE("Undocumented IM aliases select the expected interrupt mode", "[undocumented][im-alias]") {
    SECTION("IM 0 aliases") {
        const uint8_t aliases[] = {0x66};

        for (uint8_t alias : aliases) {
            CpuHarness h;
            h.cpu.int_mode = 2;
            h.load({0xed, alias});

            const StepResult step = h.step();

            INFO("opcode=0xed" << std::hex << static_cast<uint32_t>(alias));
            REQUIRE(step.cycle_delta() == 8);
            REQUIRE(h.cpu.int_mode == 0);
        }
    }

    SECTION("IM 1 aliases") {
        const uint8_t aliases[] = {0x4e, 0x6e, 0x76};

        for (uint8_t alias : aliases) {
            CpuHarness h;
            h.cpu.int_mode = 0;
            h.load({0xed, alias});

            const StepResult step = h.step();

            INFO("opcode=0xed" << std::hex << static_cast<uint32_t>(alias));
            REQUIRE(step.cycle_delta() == 8);
            REQUIRE(h.cpu.int_mode == 1);
        }
    }

    SECTION("IM 2 aliases") {
        const uint8_t aliases[] = {0x7e};

        for (uint8_t alias : aliases) {
            CpuHarness h;
            h.cpu.int_mode = 0;
            h.load({0xed, alias});

            const StepResult step = h.step();

            INFO("opcode=0xed" << std::hex << static_cast<uint32_t>(alias));
            REQUIRE(step.cycle_delta() == 8);
            REQUIRE(h.cpu.int_mode == 2);
        }
    }
}

TEST_CASE("Undocumented OUT (C),0 writes a zero byte to the selected port", "[undocumented][outc0]") {
    CpuHarness h;
    h.cpu.bc.set(0x12fe);
    h.cpu.af.accum(0xff);
    h.mem.port_254 = 0xaa;
    h.load({0xed, 0x71});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 12);
    REQUIRE(h.mem.port_254 == 0x00);
    REQUIRE(h.cpu.bc.get() == 0x12fe);
    REQUIRE(h.cpu.af.accum() == 0xff);
}

TEST_CASE("Ignored prefix combinations follow last-prefix or no-effect rules", "[undocumented][prefix]") {
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
        h.mem[0xa301] = 0x81;
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
        h.mem[0xa33f] = 0x80;
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
          "[undocumented][indexed-cb-copy]") {
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
            h.mem[0xa401] = 0x81;
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
            h.mem[0xa500] = 0xff;
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
