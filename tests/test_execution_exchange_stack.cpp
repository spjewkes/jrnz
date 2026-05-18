#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("PUSH and POP round-trip register pairs through the stack", "[exchange-stack]") {
    SECTION("PUSH/POP BC") {
        CpuHarness h;
        h.cpu.bc.set(0x1234);
        h.cpu.sp.set(0xfffe);
        h.load({0xc5, 0xc1});

        const StepResult push = h.step();
        REQUIRE(push.cycle_delta() == 11);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x1234);
        REQUIRE(h.mem[0xfffc] == 0x34);
        REQUIRE(h.mem[0xfffd] == 0x12);

        h.cpu.bc.set(0x0000);
        const StepResult pop = h.step();
        REQUIRE(pop.cycle_delta() == 10);
        REQUIRE(h.cpu.bc.get() == 0x1234);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
        REQUIRE(h.cpu.pc.get() == 0x0002);
    }

    SECTION("PUSH/POP AF") {
        CpuHarness h;
        h.cpu.af.set(0xa55a);
        h.cpu.sp.set(0xfffe);
        h.load({0xf5, 0xf1});

        const StepResult push = h.step();
        REQUIRE(push.cycle_delta() == 11);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0xa55a);
        REQUIRE(h.mem[0xfffc] == 0x5a);
        REQUIRE(h.mem[0xfffd] == 0xa5);

        h.cpu.af.set(0x0000);
        const StepResult pop = h.step();
        REQUIRE(pop.cycle_delta() == 10);
        REQUIRE(h.cpu.af.get() == 0xa55a);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
        REQUIRE(h.cpu.pc.get() == 0x0002);
    }

    SECTION("PUSH/POP IX") {
        CpuHarness h;
        h.cpu.ix.set(0xbeef);
        h.cpu.sp.set(0xfffe);
        h.load({0xdd, 0xe5, 0xdd, 0xe1});

        const StepResult push = h.step();
        REQUIRE(push.cycle_delta() == 15);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0xbeef);
        REQUIRE(h.mem[0xfffc] == 0xef);
        REQUIRE(h.mem[0xfffd] == 0xbe);

        h.cpu.ix.set(0x0000);
        const StepResult pop = h.step();
        REQUIRE(pop.cycle_delta() == 14);
        REQUIRE(h.cpu.ix.get() == 0xbeef);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
        REQUIRE(h.cpu.pc.get() == 0x0004);
    }

    SECTION("PUSH/POP DE and HL preserve flags and stack byte order") {
        CpuHarness h;
        h.cpu.af.flags(0x96);
        h.cpu.de.set(0x2468);
        h.cpu.hl.set(0x1357);
        h.cpu.sp.set(0xfffe);
        h.load({0xd5, 0xe5, 0xe1, 0xd1});

        const StepResult push_de = h.step();
        REQUIRE(push_de.cycle_delta() == 11);
        REQUIRE(push_de.pc_after == 0x0001);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.mem[0xfffc] == 0x68);
        REQUIRE(h.mem[0xfffd] == 0x24);
        REQUIRE(h.cpu.af.flags() == 0x96);

        const StepResult push_hl = h.step();
        REQUIRE(push_hl.cycle_delta() == 11);
        REQUIRE(push_hl.pc_after == 0x0002);
        REQUIRE(h.cpu.sp.get() == 0xfffa);
        REQUIRE(h.mem[0xfffa] == 0x57);
        REQUIRE(h.mem[0xfffb] == 0x13);
        REQUIRE(h.cpu.af.flags() == 0x96);

        h.cpu.hl.set(0x0000);
        const StepResult pop_hl = h.step();
        REQUIRE(pop_hl.cycle_delta() == 10);
        REQUIRE(pop_hl.pc_after == 0x0003);
        REQUIRE(h.cpu.hl.get() == 0x1357);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
        REQUIRE(h.cpu.af.flags() == 0x96);

        h.cpu.de.set(0x0000);
        const StepResult pop_de = h.step();
        REQUIRE(pop_de.cycle_delta() == 10);
        REQUIRE(pop_de.pc_after == 0x0004);
        REQUIRE(h.cpu.de.get() == 0x2468);
        REQUIRE(h.cpu.sp.get() == 0xfffe);
        REQUIRE(h.cpu.af.flags() == 0x96);
    }
}

TEST_CASE("POP then PUSH AF preserves the packed flag byte and stack ordering", "[exchange-stack]") {
    CpuHarness h;
    h.cpu.af.set(0x1234);
    h.cpu.sp.set(0xfffc);
    h.mem.write_addr_to_mem(0xfffc, 0xa55a);
    h.load({0xf1, 0xf5});

    const StepResult pop = h.step();
    REQUIRE(pop.cycle_delta() == 10);
    REQUIRE(h.cpu.af.get() == 0xa55a);
    REQUIRE(h.cpu.sp.get() == 0xfffe);

    h.mem.write_addr_to_mem(0xfffc, 0x0000);

    const StepResult push = h.step();
    REQUIRE(push.cycle_delta() == 11);
    REQUIRE(h.cpu.af.get() == 0xa55a);
    REQUIRE(h.cpu.sp.get() == 0xfffc);
    REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0xa55a);
    REQUIRE(h.mem[0xfffc] == 0x5a);
    REQUIRE(h.mem[0xfffd] == 0xa5);
    REQUIRE(h.cpu.pc.get() == 0x0002);
}

TEST_CASE("Exchange instructions swap the documented registers", "[exchange-stack]") {
    SECTION("EX DE,HL swaps DE and HL") {
        CpuHarness h;
        h.cpu.de.set(0x1234);
        h.cpu.hl.set(0xabcd);
        h.load({0xeb});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.de.get() == 0xabcd);
        REQUIRE(h.cpu.hl.get() == 0x1234);
        REQUIRE(h.cpu.pc.get() == 0x0001);
    }

    SECTION("EX AF,AF' swaps with the alternate AF register") {
        CpuHarness h;
        h.cpu.af.set(0xaaaa);
        h.cpu.af.swap();
        h.cpu.af.set(0x1234);
        h.load({0x08});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.af.get() == 0xaaaa);
        REQUIRE(h.cpu.pc.get() == 0x0001);

        h.cpu.af.swap();
        REQUIRE(h.cpu.af.get() == 0x1234);
    }

    SECTION("EXX swaps BC, DE, and HL with their alternate registers") {
        CpuHarness h;
        h.cpu.bc.set(0xb0b0);
        h.cpu.de.set(0xd0d0);
        h.cpu.hl.set(0xe0e0);
        h.cpu.bc.swap();
        h.cpu.de.swap();
        h.cpu.hl.swap();
        h.cpu.bc.set(0x1111);
        h.cpu.de.set(0x2222);
        h.cpu.hl.set(0x3333);
        h.load({0xd9});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 4);
        REQUIRE(h.cpu.bc.get() == 0xb0b0);
        REQUIRE(h.cpu.de.get() == 0xd0d0);
        REQUIRE(h.cpu.hl.get() == 0xe0e0);
        REQUIRE(h.cpu.pc.get() == 0x0001);

        h.cpu.bc.swap();
        h.cpu.de.swap();
        h.cpu.hl.swap();
        REQUIRE(h.cpu.bc.get() == 0x1111);
        REQUIRE(h.cpu.de.get() == 0x2222);
        REQUIRE(h.cpu.hl.get() == 0x3333);
    }
}

TEST_CASE("EX (SP),rr swaps stack memory with register pairs", "[exchange-stack]") {
    SECTION("EX (SP),HL") {
        CpuHarness h;
        h.cpu.hl.set(0x1234);
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0xabcd);
        h.load({0xe3});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 19);
        REQUIRE(h.cpu.hl.get() == 0xabcd);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x1234);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
    }

    SECTION("EX (SP),IX") {
        CpuHarness h;
        h.cpu.ix.set(0x1357);
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0x2468);
        h.load({0xdd, 0xe3});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.cpu.ix.get() == 0x2468);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0x1357);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
    }

    SECTION("EX (SP),IY") {
        CpuHarness h;
        h.cpu.iy.set(0xface);
        h.cpu.sp.set(0xfffc);
        h.mem.write_addr_to_mem(0xfffc, 0xbeef);
        h.load({0xfd, 0xe3});

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 23);
        REQUIRE(h.cpu.iy.get() == 0xbeef);
        REQUIRE(h.mem.read_addr_from_mem(0xfffc) == 0xface);
        REQUIRE(h.cpu.sp.get() == 0xfffc);
    }

    SECTION("EX (SP),HL reads ROM but cannot modify it") {
        CpuHarness h;
        h.cpu.hl.set(0x1234);
        h.cpu.sp.set(0x004e);
        h.mem.poke_mapped_for_test(0x004e, 0xc1);
        h.mem.poke_mapped_for_test(0x004f, 0xe1);
        h.load({0xe3}, 0x8000);

        const StepResult step = h.step();

        REQUIRE(step.cycle_delta() == 19);
        REQUIRE(h.cpu.hl.get() == 0xe1c1);
        REQUIRE(h.mem[0x004e] == 0xc1);
        REQUIRE(h.mem[0x004f] == 0xe1);
        REQUIRE(h.cpu.sp.get() == 0x004e);
    }
}
