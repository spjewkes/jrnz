#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("INI reads from the port into memory and updates registers", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x03ff);
    h.cpu.hl.set(0x8000);
    h.mem.floating_counter = 0;
    h.mem[0x4000] = 0x5a;
    h.load({0xed, 0xa2});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 16);
    REQUIRE(h.mem[0x8000] == 0x5a);
    REQUIRE(h.cpu.hl.get() == 0x8001);
    REQUIRE(h.cpu.bc.get() == 0x02ff);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Sign));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
}

TEST_CASE("INIR repeats until B becomes zero", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x02ff);
    h.cpu.hl.set(0x8100);
    h.mem.floating_counter = 0;
    h.mem[0x4000] = 0x11;
    h.mem[0x4001] = 0x22;
    h.load({0xed, 0xb2});

    const StepResult first = h.step();
    REQUIRE(first.cycle_delta() == 21);
    REQUIRE(h.cpu.pc.get() == 0x0000);
    REQUIRE(h.mem[0x8100] == 0x11);
    REQUIRE(h.cpu.hl.get() == 0x8101);
    REQUIRE(h.cpu.bc.get() == 0x01ff);

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.mem[0x8101] == 0x22);
    REQUIRE(h.cpu.hl.get() == 0x8102);
    REQUIRE(h.cpu.bc.get() == 0x00ff);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
}

TEST_CASE("IND reads from the port into memory and decrements HL", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x02ff);
    h.cpu.hl.set(0x8201);
    h.mem.floating_counter = 0;
    h.mem[0x4000] = 0x77;
    h.load({0xed, 0xaa});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 16);
    REQUIRE(h.mem[0x8201] == 0x77);
    REQUIRE(h.cpu.hl.get() == 0x8200);
    REQUIRE(h.cpu.bc.get() == 0x01ff);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
}

TEST_CASE("INDR repeats while decrementing HL until B becomes zero", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x02ff);
    h.cpu.hl.set(0x8301);
    h.mem.floating_counter = 0;
    h.mem[0x4000] = 0xc1;
    h.mem[0x4001] = 0xc2;
    h.load({0xed, 0xba});

    const StepResult first = h.step();
    REQUIRE(first.cycle_delta() == 21);
    REQUIRE(h.cpu.pc.get() == 0x0000);
    REQUIRE(h.mem[0x8301] == 0xc1);
    REQUIRE(h.cpu.hl.get() == 0x8300);
    REQUIRE(h.cpu.bc.get() == 0x01ff);

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.mem[0x8300] == 0xc2);
    REQUIRE(h.cpu.hl.get() == 0x82ff);
    REQUIRE(h.cpu.bc.get() == 0x00ff);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
}

TEST_CASE("OUTI writes memory to the port and increments HL", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x02fe);
    h.cpu.hl.set(0x9000);
    h.mem[0x9000] = 0x12;
    h.load({0xed, 0xa3});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 16);
    REQUIRE(h.mem.port_254 == 0x12);
    REQUIRE(h.cpu.hl.get() == 0x9001);
    REQUIRE(h.cpu.bc.get() == 0x01fe);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
}

TEST_CASE("OTIR repeats until B becomes zero", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x02fe);
    h.cpu.hl.set(0x9100);
    h.mem[0x9100] = 0x12;
    h.mem[0x9101] = 0x34;
    h.load({0xed, 0xb3});

    const StepResult first = h.step();
    REQUIRE(first.cycle_delta() == 21);
    REQUIRE(h.cpu.pc.get() == 0x0000);
    REQUIRE(h.mem.port_254 == 0x12);
    REQUIRE(h.cpu.hl.get() == 0x9101);
    REQUIRE(h.cpu.bc.get() == 0x01fe);

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.mem.port_254 == 0x34);
    REQUIRE(h.cpu.hl.get() == 0x9102);
    REQUIRE(h.cpu.bc.get() == 0x00fe);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
}

TEST_CASE("OUTD writes memory to the port and decrements HL", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x02fe);
    h.cpu.hl.set(0x9201);
    h.mem[0x9201] = 0xab;
    h.load({0xed, 0xab});

    const StepResult step = h.step();

    REQUIRE(step.cycle_delta() == 16);
    REQUIRE(h.mem.port_254 == 0xab);
    REQUIRE(h.cpu.hl.get() == 0x9200);
    REQUIRE(h.cpu.bc.get() == 0x01fe);
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::Carry));
}

TEST_CASE("OTDR repeats while decrementing HL until B becomes zero", "[block-io]") {
    CpuHarness h;
    h.cpu.bc.set(0x02fe);
    h.cpu.hl.set(0x9301);
    h.mem[0x9301] = 0xde;
    h.mem[0x9300] = 0xad;
    h.load({0xed, 0xbb});

    const StepResult first = h.step();
    REQUIRE(first.cycle_delta() == 21);
    REQUIRE(h.cpu.pc.get() == 0x0000);
    REQUIRE(h.mem.port_254 == 0xde);
    REQUIRE(h.cpu.hl.get() == 0x9300);
    REQUIRE(h.cpu.bc.get() == 0x01fe);

    const StepResult second = h.step();
    REQUIRE(second.cycle_delta() == 16);
    REQUIRE(h.cpu.pc.get() == 0x0002);
    REQUIRE(h.mem.port_254 == 0xad);
    REQUIRE(h.cpu.hl.get() == 0x92ff);
    REQUIRE(h.cpu.bc.get() == 0x00fe);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Zero));
    REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::ParityOverflow));
}
