#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
#include "test_support.hpp"
#include "z80.hpp"

TEST_CASE("CP edge cases update all documented flags without modifying A", "[flags][compliance][flags-matrix]") {
    Bus mem(65536);
    Z80 state(mem, true);
    Instruction cp(InstType::CP, "cp *", 0, 0, Operand::A, Operand::N);

    struct CpCase {
        uint8_t a;
        uint8_t operand;
        uint8_t flags;
    };

    const CpCase cases[] = {
        {0x00, 0x00, 0x42}, {0x10, 0x01, 0x1a}, {0x00, 0x01, 0xbb},
        {0x80, 0x01, 0x3e}, {0x7f, 0xff, 0x87}, {0x28, 0x08, 0x22},
    };

    for (const auto &test : cases) {
        state.af.accum(test.a);
        state.af.flags(0);
        uint8_t a_after = test.a;
        StorageElement dst(&a_after, 1);
        StorageElement src(test.operand);

        cp.do_cp(state, dst, src);

        INFO("a=0x" << std::hex << static_cast<unsigned int>(test.a) << " operand=0x"
                    << static_cast<unsigned int>(test.operand));
        REQUIRE(a_after == test.a);
        require_flags(state.af.flags(), test.flags);
    }
}

TEST_CASE("DAA edge cases update all documented flags", "[flags][compliance][flags-matrix]") {
    Bus mem(65536);
    Z80 state(mem, true);
    Instruction daa(InstType::DAA, "daa", 0, 0, Operand::A, Operand::A);

    struct DaaCase {
        uint8_t input;
        uint8_t initial_flags;
        uint8_t result;
        uint8_t flags;
    };

    const DaaCase cases[] = {
        {0x3c, 0x00, 0x42, 0x14}, {0x9a, 0x00, 0x00, 0x55}, {0x0f, 0x02, 0x09, 0x0e}, {0x73, 0x03, 0x13, 0x03},
        {0x7d, 0x00, 0x83, 0x90}, {0x12, 0x00, 0x12, 0x04}, {0x1a, 0x00, 0x20, 0x30}, {0x90, 0x01, 0xf0, 0xa5},
        {0xfa, 0x00, 0x60, 0x35}, {0xa0, 0x01, 0x00, 0x45}, {0x15, 0x02, 0x15, 0x02}, {0x9f, 0x02, 0x39, 0x2f},
        {0xff, 0x03, 0x99, 0x8f}, {0x0f, 0x12, 0x09, 0x0e}, {0x13, 0x12, 0x0d, 0x1a}, {0x73, 0x13, 0x0d, 0x1b},
        {0x00, 0x13, 0x9a, 0x9f}, {0x66, 0x03, 0x06, 0x07},
    };

    for (const auto &test : cases) {
        state.af.accum(test.input);
        state.af.flags(test.initial_flags);
        uint8_t result = test.input;
        StorageElement dst(&result, 1);

        daa.do_daa(state, dst, dst);

        INFO("a=0x" << std::hex << static_cast<unsigned int>(test.input) << " initial_flags=0x"
                    << static_cast<unsigned int>(test.initial_flags));
        REQUIRE(result == test.result);
        require_flags(state.af.flags(), test.flags);
    }
}

TEST_CASE("BIT updates status but preserves carry", "[flags][compliance][flags-matrix]") {
    Bus mem(65536);
    Z80 state(mem, true);
    Instruction bit(InstType::BIT, "bit", 0, 0, Operand::A, Operand::UNUSED);

    struct BitCase {
        uint8_t value;
        uint8_t bit_index;
        uint8_t flags;
    };

    const BitCase cases[] = {
        {0x80, 7, 0x91},
        {0x00, 3, 0x55},
        {0x28, 3, 0x39},
        {0x08, 7, 0x5d},
    };

    for (const auto &test : cases) {
        state.af.flags(0);
        state.af.flag(RegisterAF::Flags::Carry, true);
        uint8_t value = test.value;
        StorageElement dst(&value, 1);
        StorageElement bit_index(test.bit_index);

        bit.do_bit(state, dst, bit_index);

        INFO("value=0x" << std::hex << static_cast<unsigned int>(test.value)
                        << " bit=" << static_cast<unsigned int>(test.bit_index));
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry));
        require_flags(state.af.flags(), test.flags);
    }
}

TEST_CASE("INC and DEC preserve carry while updating other arithmetic flags", "[flags][compliance][flags-matrix]") {
    Bus mem(65536);
    Z80 state(mem, true);

    Instruction inc(InstType::INC, "inc", 0, 0, Operand::A, Operand::ONE);
    Instruction dec(InstType::DEC, "dec", 0, 0, Operand::A, Operand::ONE);
    StorageElement one(1);

    struct ArithCase {
        uint8_t input;
        uint8_t result;
        uint8_t flags;
    };

    const ArithCase inc_cases[] = {
        {0x0f, 0x10, 0x11},
        {0x7f, 0x80, 0x95},
        {0xff, 0x00, 0x51},
        {0x1f, 0x20, 0x31},
    };

    for (const auto &test : inc_cases) {
        state.af.flags(0);
        state.af.flag(RegisterAF::Flags::Carry, true);
        uint8_t value = test.input;
        StorageElement dst(&value, 1);

        inc.do_inc(state, dst, one);

        INFO("inc input=0x" << std::hex << static_cast<unsigned int>(test.input));
        REQUIRE(value == test.result);
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry));
        require_flags(state.af.flags(), test.flags);
    }

    const ArithCase dec_cases[] = {
        {0x10, 0x0f, 0x1b},
        {0x80, 0x7f, 0x3f},
        {0x01, 0x00, 0x43},
        {0x20, 0x1f, 0x1b},
    };

    for (const auto &test : dec_cases) {
        state.af.flags(0);
        state.af.flag(RegisterAF::Flags::Carry, true);
        uint8_t value = test.input;
        StorageElement dst(&value, 1);

        dec.do_dec(state, dst, one);

        INFO("dec input=0x" << std::hex << static_cast<unsigned int>(test.input));
        REQUIRE(value == test.result);
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry));
        require_flags(state.af.flags(), test.flags);
    }
}
