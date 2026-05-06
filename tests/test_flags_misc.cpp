#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
#include "test_support.hpp"
#include "z80.hpp"

TEST_CASE("NEG handles zero, borrow, and overflow edges", "[flags]") {
    Bus mem(65536);
    Z80 state(mem, true);

    struct NegCase {
        uint8_t input;
        uint8_t result;
        bool carry;
        bool overflow;
        bool zero;
        bool sign;
    };

    const NegCase cases[] = {
        {0x00, 0x00, false, false, true, false},
        {0x01, 0xff, true, false, false, true},
        {0x80, 0x80, true, true, false, true},
    };

    Instruction neg(InstType::NEG, "neg", 0, 0, Operand::A, Operand::A);
    for (const auto &test : cases) {
        state.af.flags(0);
        uint8_t result = test.input;
        StorageElement dst(&result, 1);
        StorageElement src(test.input);

        neg.do_neg(state, dst, src);

        INFO("input=0x" << std::hex << static_cast<uint32_t>(test.input));
        REQUIRE(result == test.result);
        REQUIRE(state.af.flag(RegisterAF::Flags::AddSubtract));
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry) == test.carry);
        REQUIRE(state.af.flag(RegisterAF::Flags::ParityOverflow) == test.overflow);
        REQUIRE(state.af.flag(RegisterAF::Flags::Zero) == test.zero);
        REQUIRE(state.af.flag(RegisterAF::Flags::Sign) == test.sign);
    }
}

TEST_CASE("DAA corrects both addition and subtraction BCD results", "[flags]") {
    Bus mem(65536);
    Z80 state(mem, true);

    Instruction add(InstType::ADD, "add a,*", 0, 0, Operand::A, Operand::N);
    Instruction sub(InstType::SUB, "sub *", 0, 0, Operand::A, Operand::N);
    Instruction daa(InstType::DAA, "daa", 0, 0, Operand::A, Operand::A);

    uint8_t add_result = 0x15;
    StorageElement add_dst(&add_result, 1);
    StorageElement add_src(0x27);
    add.do_add(state, add_dst, add_src);
    REQUIRE(add_result == 0x3c);

    daa.do_daa(state, add_dst, add_dst);
    REQUIRE(add_result == 0x42);
    REQUIRE_FALSE(state.af.flag(RegisterAF::Flags::Carry));
    REQUIRE_FALSE(state.af.flag(RegisterAF::Flags::AddSubtract));

    state.af.flags(0);
    uint8_t sub_result = 0x15;
    StorageElement sub_dst(&sub_result, 1);
    StorageElement sub_src(0x06);
    sub.do_sub(state, sub_dst, sub_src);
    REQUIRE(sub_result == 0x0f);
    REQUIRE(state.af.flag(RegisterAF::Flags::AddSubtract));

    daa.do_daa(state, sub_dst, sub_dst);
    REQUIRE(sub_result == 0x09);
    REQUIRE(state.af.flag(RegisterAF::Flags::AddSubtract));
    REQUIRE_FALSE(state.af.flag(RegisterAF::Flags::Carry));
}
