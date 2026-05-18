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
        REQUIRE(h.peek(0x8ffe) == 0xfe);
        REQUIRE(h.peek(0x8fff) == 0xca);

        h.cpu.iy.set(0x0000);
        const StepResult pop = h.step();
        REQUIRE(pop.cycle_delta() == 14);
        REQUIRE(pop.pc_after == 0x0004);
        REQUIRE(h.cpu.iy.get() == 0xcafe);
        REQUIRE(h.cpu.sp.get() == 0x9000);
        REQUIRE(h.cpu.af.flags() == 0x5a);
    }

    SECTION("PUSH and POP IX preserve the neighboring indexed register and flags") {
        CpuHarness h;
        h.cpu.ix.set(0x1234);
        h.cpu.iy.set(0xabcd);
        h.cpu.sp.set(0x9100);
        h.cpu.af.flags(0x3c);
        h.load({0xdd, 0xe5, 0xdd, 0xe1});

        const StepResult push = h.step();
        REQUIRE(push.cycle_delta() == 15);
        REQUIRE(push.pc_after == 0x0002);
        REQUIRE(h.cpu.sp.get() == 0x90fe);
        REQUIRE(h.peek(0x90fe) == 0x34);
        REQUIRE(h.peek(0x90ff) == 0x12);
        REQUIRE(h.cpu.iy.get() == 0xabcd);
        REQUIRE(h.cpu.af.flags() == 0x3c);

        h.cpu.ix.set(0x0000);
        const StepResult pop = h.step();
        REQUIRE(pop.cycle_delta() == 14);
        REQUIRE(pop.pc_after == 0x0004);
        REQUIRE(h.cpu.ix.get() == 0x1234);
        REQUIRE(h.cpu.iy.get() == 0xabcd);
        REQUIRE(h.cpu.sp.get() == 0x9100);
        REQUIRE(h.cpu.af.flags() == 0x3c);
    }

    SECTION("Indexed pair arithmetic and increment-decrement preserve the documented flag subset") {
        CpuHarness h;
        h.cpu.af.flags(0xa5);
        h.cpu.ix.set(0x8fff);
        h.cpu.iy.set(0xffff);
        h.load({0xdd, 0x29, 0xfd, 0x29, 0xdd, 0x23, 0xfd, 0x2b});

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

        const uint8_t flags_before_inc_dec = h.cpu.af.flags();
        const StepResult inc_ix = h.step();
        REQUIRE(inc_ix.cycle_delta() == 10);
        REQUIRE(inc_ix.pc_after == 0x0006);
        REQUIRE(h.cpu.ix.get() == 0x1fff);
        REQUIRE(h.cpu.af.flags() == flags_before_inc_dec);

        const StepResult dec_iy = h.step();
        REQUIRE(dec_iy.cycle_delta() == 10);
        REQUIRE(dec_iy.pc_after == 0x0008);
        REQUIRE(h.cpu.iy.get() == 0xfffd);
        REQUIRE(h.cpu.af.flags() == flags_before_inc_dec);
    }
}
