#include <catch2/catch_test_macros.hpp>

#include "test_support.hpp"

TEST_CASE("Indexed pair control and stack forms use IX and IY exactly as documented", "[index-pairs]") {
    SECTION("JP (IX) and JP (IY) jump through the indexed pair without reading immediates") {
        CpuHarness h;
        h.cpu.ix.set(0x8123);
        h.cpu.iy.set(0x9abc);
        h.load({0xdd, 0xe9, 0xfd, 0xe9});

        const StepResult jp_ix = h.step();
        REQUIRE(jp_ix.cycle_delta() == 8);
        REQUIRE(jp_ix.pc_after == 0x8123);
        REQUIRE(h.cpu.ix.get() == 0x8123);
        REQUIRE(h.cpu.iy.get() == 0x9abc);

        h.cpu.pc.set(0x0002);
        const StepResult jp_iy = h.step();
        REQUIRE(jp_iy.cycle_delta() == 8);
        REQUIRE(jp_iy.pc_after == 0x9abc);
        REQUIRE(h.cpu.ix.get() == 0x8123);
        REQUIRE(h.cpu.iy.get() == 0x9abc);
    }

    SECTION("LD SP,IX and LD SP,IY copy the full 16-bit indexed pair without disturbing flags") {
        CpuHarness h;
        h.cpu.af.flags(0xa5);
        h.cpu.ix.set(0x4567);
        h.cpu.iy.set(0x89ab);
        h.cpu.sp.set(0x1111);
        h.load({0xdd, 0xf9, 0xfd, 0xf9});

        const StepResult ld_sp_ix = h.step();
        REQUIRE(ld_sp_ix.cycle_delta() == 10);
        REQUIRE(ld_sp_ix.pc_after == 0x0002);
        REQUIRE(h.cpu.sp.get() == 0x4567);
        REQUIRE(h.cpu.ix.get() == 0x4567);
        REQUIRE(h.cpu.af.flags() == 0xa5);

        const StepResult ld_sp_iy = h.step();
        REQUIRE(ld_sp_iy.cycle_delta() == 10);
        REQUIRE(ld_sp_iy.pc_after == 0x0004);
        REQUIRE(h.cpu.sp.get() == 0x89ab);
        REQUIRE(h.cpu.iy.get() == 0x89ab);
        REQUIRE(h.cpu.af.flags() == 0xa5);
    }

    SECTION("PUSH and POP IY round-trip the indexed register pair through memory in little-endian order") {
        CpuHarness h;
        h.cpu.iy.set(0xcafe);
        h.cpu.sp.set(0x9000);
        h.cpu.af.flags(0x5a);
        h.load({0xfd, 0xe5, 0xfd, 0xe1});

        const StepResult push = h.step();
        REQUIRE(push.cycle_delta() == 15);
        REQUIRE(push.pc_after == 0x0002);
        REQUIRE(h.cpu.sp.get() == 0x8ffe);
        REQUIRE(h.mem[0x8ffe] == 0xfe);
        REQUIRE(h.mem[0x8fff] == 0xca);

        h.cpu.iy.set(0x0000);
        const StepResult pop = h.step();
        REQUIRE(pop.cycle_delta() == 14);
        REQUIRE(pop.pc_after == 0x0004);
        REQUIRE(h.cpu.iy.get() == 0xcafe);
        REQUIRE(h.cpu.sp.get() == 0x9000);
        REQUIRE(h.cpu.af.flags() == 0x5a);
    }

    SECTION("ADD IX,IX and ADD IY,IY use indexed 16-bit half-carry and carry rules") {
        CpuHarness h;
        h.cpu.af.flags(0x00);
        h.cpu.ix.set(0x8fff);
        h.cpu.iy.set(0xffff);
        h.load({0xdd, 0x29, 0xfd, 0x29});

        const StepResult add_ix_ix = h.step();
        REQUIRE(add_ix_ix.cycle_delta() == 15);
        REQUIRE(add_ix_ix.pc_after == 0x0002);
        REQUIRE(h.cpu.ix.get() == 0x1ffe);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));

        const StepResult add_iy_iy = h.step();
        REQUIRE(add_iy_iy.cycle_delta() == 15);
        REQUIRE(add_iy_iy.pc_after == 0x0004);
        REQUIRE(h.cpu.iy.get() == 0xfffe);
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::Carry));
        REQUIRE(h.cpu.af.flag(RegisterAF::Flags::HalfCarry));
        REQUIRE_FALSE(h.cpu.af.flag(RegisterAF::Flags::AddSubtract));
    }
}
