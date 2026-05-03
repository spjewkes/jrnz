#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Documented load instructions move bytes and words without disturbing flags", "[loads]") {
    SECTION("LD rr,nn loads immediate 16-bit values") {
        CpuHarness h;
        h.cpu.af.flags(0xa5);
        h.load({0x01, 0x34, 0x12, 0x31, 0x78, 0x56});

        const StepResult bc = h.step();
        REQUIRE(bc.cycle_delta() == 10);
        REQUIRE(bc.pc_after == 0x0003);
        REQUIRE(h.cpu.bc.get() == 0x1234);
        REQUIRE(h.cpu.af.flags() == 0xa5);

        const StepResult sp = h.step();
        REQUIRE(sp.cycle_delta() == 10);
        REQUIRE(sp.pc_after == 0x0006);
        REQUIRE(h.cpu.sp.get() == 0x5678);
        REQUIRE(h.cpu.af.flags() == 0xa5);
    }

    SECTION("LD r,r and LD r,(HL) transfer 8-bit values") {
        CpuHarness h;
        h.cpu.af.flags(0x53);
        h.cpu.bc.hi(0x12);
        h.cpu.hl.set(0x8700);
        h.mem[0x8700] = 0x9a;
        h.load({0x48, 0x7e, 0x70});

        const StepResult reg = h.step();
        REQUIRE(reg.cycle_delta() == 4);
        REQUIRE(reg.pc_after == 0x0001);
        REQUIRE(h.cpu.bc.lo() == 0x12);
        REQUIRE(h.cpu.af.flags() == 0x53);

        const StepResult from_mem = h.step();
        REQUIRE(from_mem.cycle_delta() == 7);
        REQUIRE(from_mem.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0x9a);
        REQUIRE(h.cpu.af.flags() == 0x53);

        h.cpu.bc.hi(0x44);
        const StepResult to_mem = h.step();
        REQUIRE(to_mem.cycle_delta() == 7);
        REQUIRE(to_mem.pc_after == 0x0003);
        REQUIRE(h.mem[0x8700] == 0x44);
        REQUIRE(h.cpu.af.flags() == 0x53);
    }

    SECTION("LD (BC),A and LD A,(DE) use the documented address registers") {
        CpuHarness h;
        h.cpu.af.set(0x5ac3);
        h.cpu.bc.set(0x8800);
        h.cpu.de.set(0x8801);
        h.mem[0x8801] = 0xa7;
        h.load({0x02, 0x1a});

        const StepResult store = h.step();
        REQUIRE(store.cycle_delta() == 7);
        REQUIRE(store.pc_after == 0x0001);
        REQUIRE(h.mem[0x8800] == 0x5a);
        REQUIRE(h.cpu.af.flags() == 0xc3);

        const StepResult load = h.step();
        REQUIRE(load.cycle_delta() == 7);
        REQUIRE(load.pc_after == 0x0002);
        REQUIRE(h.cpu.af.accum() == 0xa7);
        REQUIRE(h.cpu.af.flags() == 0xc3);
    }

    SECTION("LD (nn),A and LD A,(nn) use little-endian addresses") {
        CpuHarness h;
        h.cpu.af.set(0x3ca5);
        h.mem[0x8942] = 0xe1;
        h.load({0x32, 0x40, 0x89, 0x3a, 0x42, 0x89});

        const StepResult store = h.step();
        REQUIRE(store.cycle_delta() == 13);
        REQUIRE(store.pc_after == 0x0003);
        REQUIRE(h.mem[0x8940] == 0x3c);
        REQUIRE(h.cpu.af.flags() == 0xa5);

        const StepResult load = h.step();
        REQUIRE(load.cycle_delta() == 13);
        REQUIRE(load.pc_after == 0x0006);
        REQUIRE(h.cpu.af.accum() == 0xe1);
        REQUIRE(h.cpu.af.flags() == 0xa5);
    }
}

TEST_CASE("Extended and indexed load instructions preserve byte order and target registers", "[loads]") {
    SECTION("LD (nn),HL and LD HL,(nn)") {
        CpuHarness h;
        h.cpu.af.flags(0x96);
        h.cpu.hl.set(0x1234);
        h.mem[0x8a10] = 0xcd;
        h.mem[0x8a11] = 0xab;
        h.load({0x22, 0x00, 0x8a, 0x2a, 0x10, 0x8a});

        const StepResult store = h.step();
        REQUIRE(store.cycle_delta() == 16);
        REQUIRE(store.pc_after == 0x0003);
        REQUIRE(h.mem.read_addr_from_mem(0x8a00) == 0x1234);
        REQUIRE(h.cpu.af.flags() == 0x96);

        const StepResult load = h.step();
        REQUIRE(load.cycle_delta() == 16);
        REQUIRE(load.pc_after == 0x0006);
        REQUIRE(h.cpu.hl.get() == 0xabcd);
        REQUIRE(h.cpu.af.flags() == 0x96);
    }

    SECTION("LD (nn),BC and LD DE,(nn) use ED-prefixed word transfers") {
        CpuHarness h;
        h.cpu.af.flags(0x69);
        h.cpu.bc.set(0xbeef);
        h.mem[0x8b20] = 0x44;
        h.mem[0x8b21] = 0x33;
        h.load({0xed, 0x43, 0x00, 0x8b, 0xed, 0x5b, 0x20, 0x8b});

        const StepResult store = h.step();
        REQUIRE(store.cycle_delta() == 20);
        REQUIRE(store.pc_after == 0x0004);
        REQUIRE(h.mem.read_addr_from_mem(0x8b00) == 0xbeef);
        REQUIRE(h.cpu.af.flags() == 0x69);

        const StepResult load = h.step();
        REQUIRE(load.cycle_delta() == 20);
        REQUIRE(load.pc_after == 0x0008);
        REQUIRE(h.cpu.de.get() == 0x3344);
        REQUIRE(h.cpu.af.flags() == 0x69);
    }

    SECTION("ED-prefixed absolute word transfers cover documented DE and SP forms") {
        CpuHarness h;
        h.cpu.af.flags(0x42);
        h.cpu.de.set(0x5678);
        h.cpu.hl.set(0xaaaa);
        h.cpu.sp.set(0x9abc);
        h.mem[0x8b50] = 0xfe;
        h.mem[0x8b51] = 0xdc;
        h.load({0xed, 0x53, 0x00, 0x8b, 0xed, 0x73, 0x10, 0x8b, 0xed, 0x7b, 0x50, 0x8b});

        const StepResult store_de = h.step();
        REQUIRE(store_de.cycle_delta() == 20);
        REQUIRE(store_de.pc_after == 0x0004);
        REQUIRE(h.mem.read_addr_from_mem(0x8b00) == 0x5678);
        REQUIRE(h.cpu.af.flags() == 0x42);

        const StepResult store_sp = h.step();
        REQUIRE(store_sp.cycle_delta() == 20);
        REQUIRE(store_sp.pc_after == 0x0008);
        REQUIRE(h.mem.read_addr_from_mem(0x8b10) == 0x9abc);
        REQUIRE(h.cpu.hl.get() == 0xaaaa);
        REQUIRE(h.cpu.af.flags() == 0x42);

        h.cpu.sp.set(0x1111);
        const StepResult load_sp = h.step();
        REQUIRE(load_sp.cycle_delta() == 20);
        REQUIRE(load_sp.pc_after == 0x000c);
        REQUIRE(h.cpu.sp.get() == 0xdcfe);
        REQUIRE(h.cpu.hl.get() == 0xaaaa);
        REQUIRE(h.cpu.af.flags() == 0x42);
    }

    SECTION("LD SP,HL copies the register pair without affecting flags") {
        CpuHarness h;
        h.cpu.af.flags(0x3c);
        h.cpu.hl.set(0x9abc);
        h.cpu.sp.set(0x1111);
        h.load({0xf9});

        const StepResult step = h.step();
        REQUIRE(step.cycle_delta() == 6);
        REQUIRE(step.pc_after == 0x0001);
        REQUIRE(h.cpu.sp.get() == 0x9abc);
        REQUIRE(h.cpu.hl.get() == 0x9abc);
        REQUIRE(h.cpu.af.flags() == 0x3c);
    }

    SECTION("LD (nn),IX and LD IY,(nn) use indexed 16-bit transfers") {
        CpuHarness h;
        h.cpu.af.flags(0xc3);
        h.cpu.ix.set(0x1357);
        h.mem[0x8c30] = 0x68;
        h.mem[0x8c31] = 0x24;
        h.load({0xdd, 0x22, 0x00, 0x8c, 0xfd, 0x2a, 0x30, 0x8c});

        const StepResult store = h.step();
        REQUIRE(store.cycle_delta() == 20);
        REQUIRE(store.pc_after == 0x0004);
        REQUIRE(h.mem.read_addr_from_mem(0x8c00) == 0x1357);
        REQUIRE(h.cpu.af.flags() == 0xc3);

        const StepResult load = h.step();
        REQUIRE(load.cycle_delta() == 20);
        REQUIRE(load.pc_after == 0x0008);
        REQUIRE(h.cpu.iy.get() == 0x2468);
        REQUIRE(h.cpu.af.flags() == 0xc3);
    }

    SECTION("Indexed absolute word transfers cover both IX and IY load-store directions") {
        CpuHarness h;
        h.cpu.af.flags(0x81);
        h.cpu.iy.set(0x9abc);
        h.mem[0x8c40] = 0x78;
        h.mem[0x8c41] = 0x56;
        h.load({0xfd, 0x22, 0x10, 0x8c, 0xdd, 0x2a, 0x40, 0x8c});

        const StepResult store_iy = h.step();
        REQUIRE(store_iy.cycle_delta() == 20);
        REQUIRE(store_iy.pc_after == 0x0004);
        REQUIRE(h.mem.read_addr_from_mem(0x8c10) == 0x9abc);
        REQUIRE(h.cpu.af.flags() == 0x81);

        const StepResult load_ix = h.step();
        REQUIRE(load_ix.cycle_delta() == 20);
        REQUIRE(load_ix.pc_after == 0x0008);
        REQUIRE(h.cpu.ix.get() == 0x5678);
        REQUIRE(h.cpu.iy.get() == 0x9abc);
        REQUIRE(h.cpu.af.flags() == 0x81);
    }
}
