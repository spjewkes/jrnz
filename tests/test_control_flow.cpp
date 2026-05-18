#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
#include "z80.hpp"

TEST_CASE("CALL and RET round-trip the return address through the stack", "[control]") {
    Bus mem(65536);
    Z80 state(mem, true);

    state.pc.set(0x1234);
    state.sp.set(0xfffe);

    Instruction call(InstType::CALL, "call", 3, 17, Conditional::ALWAYS, Operand::PC, Operand::NN);
    StorageElement pc = state.pc.element();
    StorageElement target(static_cast<uint8_t>(0xcd), static_cast<uint8_t>(0xab));

    REQUIRE(call.do_call(state, pc, target) == 17);
    REQUIRE(state.pc.get() == 0xabcd);
    REQUIRE(state.sp.get() == 0xfffc);
    REQUIRE(mem.read_addr_from_mem(0xfffc) == 0x1234);

    Instruction ret(InstType::RET, "ret", 1, 10, Conditional::ALWAYS, Operand::PC);
    StorageElement unused;

    REQUIRE(ret.do_ret(state, pc, unused) == 10);
    REQUIRE(state.pc.get() == 0x1234);
    REQUIRE(state.sp.get() == 0xfffe);
}

TEST_CASE("JR uses signed displacements and conditional timing", "[control]") {
    Bus mem(65536);
    Z80 state(mem, true);

    state.pc.set(0x2000);
    Instruction jr_taken(InstType::JR, "jr z,*", 2, 12, 7, Conditional::Z, Operand::PC, Operand::N);
    StorageElement pc = state.pc.element();
    StorageElement backward_offset(static_cast<uint8_t>(0xfe));

    state.af.flag(RegisterAF::Flags::Zero, true);
    REQUIRE(jr_taken.do_jr(state, pc, backward_offset) == 12);
    REQUIRE(state.pc.get() == 0x1ffe);

    state.pc.set(0x2000);
    state.af.flag(RegisterAF::Flags::Zero, false);
    REQUIRE(jr_taken.do_jr(state, pc, backward_offset) == 7);
    REQUIRE(state.pc.get() == 0x2000);
}

TEST_CASE("DJNZ decrements B and only branches while non-zero", "[control]") {
    Bus mem(65536);
    Z80 state(mem, true);

    Instruction djnz(InstType::DJNZ, "djnz *", 2, 13, 8, Conditional::NZ, Operand::PC, Operand::N);
    StorageElement pc = state.pc.element();
    StorageElement offset(static_cast<uint8_t>(0x05));

    state.pc.set(0x0100);
    state.bc.hi(0x02);
    REQUIRE(djnz.do_djnz(state, pc, offset) == 13);
    REQUIRE(state.bc.hi() == 0x01);
    REQUIRE(state.pc.get() == 0x0105);

    state.pc.set(0x0100);
    state.bc.hi(0x01);
    REQUIRE(djnz.do_djnz(state, pc, offset) == 8);
    REQUIRE(state.bc.hi() == 0x00);
    REQUIRE(state.pc.get() == 0x0100);
}

TEST_CASE("EI only enables maskable interrupts after the following instruction", "[control]") {
    Bus mem(65536);
    Z80 state(mem, true);

    mem.poke_mapped_for_test(0x0000, 0xfb);  // ei
    mem.poke_mapped_for_test(0x0001, 0x00);  // nop

    state.pc.set(0x0000);
    state.sp.set(0xfffe);
    state.interrupt = true;
    state.int_mode = 1;

    REQUIRE(state.clock());
    REQUIRE(state.pc.get() == 0x0001);
    REQUIRE_FALSE(state.iff1);
    REQUIRE(state.ei_pending);

    REQUIRE(state.clock());
    REQUIRE(state.pc.get() == 0x0002);
    REQUIRE(state.iff1);
    REQUIRE(state.iff2);
    REQUIRE_FALSE(state.ei_pending);

    REQUIRE(state.clock());
    REQUIRE(state.pc.get() == 0x0038);
    REQUIRE_FALSE(state.interrupt);
    REQUIRE(state.sp.get() == 0xfffc);
    REQUIRE(mem.read_addr_from_mem(0xfffc) == 0x0002);
}
