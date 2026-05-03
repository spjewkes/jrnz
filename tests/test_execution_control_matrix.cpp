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
