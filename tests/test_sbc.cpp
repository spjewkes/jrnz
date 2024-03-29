#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
#include "z80.hpp"

TEST_CASE("Carry Overflow Sub", "[sbc]") {
    Bus mem(65536);
    Z80 state(mem, true);

    typedef struct test_data {
        uint8_t op1;
        uint8_t op2;
        uint8_t carry_in;
        uint8_t result;
        uint8_t carry_out;
        uint8_t overflow;
    } test_data;

    test_data sbc_tests[] = {
        {0, 0, 0, 0, 0, 0},       {0, 1, 0, 255, 1, 0},     {0, 127, 0, 129, 1, 0},   {0, 128, 0, 128, 1, 1},
        {0, 129, 0, 127, 1, 0},   {0, 255, 0, 1, 1, 0},     {1, 0, 0, 1, 0, 0},       {1, 1, 0, 0, 0, 0},
        {1, 127, 0, 130, 1, 0},   {1, 128, 0, 129, 1, 1},   {1, 129, 0, 128, 1, 1},   {1, 255, 0, 2, 1, 0},
        {127, 0, 0, 127, 0, 0},   {127, 1, 0, 126, 0, 0},   {127, 127, 0, 0, 0, 0},   {127, 128, 0, 255, 1, 1},
        {127, 129, 0, 254, 1, 1}, {127, 255, 0, 128, 1, 1}, {128, 0, 0, 128, 0, 0},   {128, 1, 0, 127, 0, 1},
        {128, 127, 0, 1, 0, 1},   {128, 128, 0, 0, 0, 0},   {128, 129, 0, 255, 1, 0}, {128, 255, 0, 129, 1, 0},
        {129, 0, 0, 129, 0, 0},   {129, 1, 0, 128, 0, 0},   {129, 127, 0, 2, 0, 1},   {129, 128, 0, 1, 0, 0},
        {129, 129, 0, 0, 0, 0},   {129, 255, 0, 130, 1, 0}, {255, 0, 0, 255, 0, 0},   {255, 1, 0, 254, 0, 0},
        {255, 127, 0, 128, 0, 0}, {255, 128, 0, 127, 0, 0}, {255, 129, 0, 126, 0, 0}, {255, 255, 0, 0, 0, 0},
        {0, 0, 1, 255, 1, 0},     {0, 1, 1, 254, 1, 0},     {0, 127, 1, 128, 1, 0},   {0, 128, 1, 127, 1, 0},
        {0, 129, 1, 126, 1, 0},   {0, 255, 1, 0, 1, 0},     {1, 0, 1, 0, 0, 0},       {1, 1, 1, 255, 1, 0},
        {1, 127, 1, 129, 1, 0},   {1, 128, 1, 128, 1, 1},   {1, 129, 1, 127, 1, 0},   {1, 255, 1, 1, 1, 0},
        {127, 0, 1, 126, 0, 0},   {127, 1, 1, 125, 0, 0},   {127, 127, 1, 255, 1, 0}, {127, 128, 1, 254, 1, 1},
        {127, 129, 1, 253, 1, 1}, {127, 255, 1, 127, 1, 0}, {128, 0, 1, 127, 0, 1},   {128, 1, 1, 126, 0, 1},
        {128, 127, 1, 0, 0, 1},   {128, 128, 1, 255, 1, 0}, {128, 129, 1, 254, 1, 0}, {128, 255, 1, 128, 1, 0},
        {129, 0, 1, 128, 0, 0},   {129, 1, 1, 127, 0, 1},   {129, 127, 1, 1, 0, 1},   {129, 128, 1, 0, 0, 0},
        {129, 129, 1, 255, 1, 0}, {129, 255, 1, 129, 1, 0}, {255, 0, 1, 254, 0, 0},   {255, 1, 1, 253, 0, 0},
        {255, 127, 1, 127, 0, 1}, {255, 128, 1, 126, 0, 0}, {255, 129, 1, 125, 0, 0}, {255, 255, 1, 255, 1, 0},
    };

    size_t length = sizeof(sbc_tests) / sizeof(sbc_tests[0]);
    for (size_t i = 0; i < length; i++) {
        state.af.flags(0);
        uint8_t result = sbc_tests[i].op1;
        StorageElement dst = StorageElement(&result, 1);
        StorageElement src = StorageElement(sbc_tests[i].op2);

        state.af.flag(RegisterAF::Flags::Carry, (sbc_tests[i].carry_in ? true : false));

        Instruction instruction = Instruction(InstType::SBC, "test", 0, 0);
        instruction.do_sbc(state, dst, src);

        INFO("Calculating [" << i << "]: " << static_cast<uint32_t>(sbc_tests[i].op1) << " - "
                             << static_cast<uint32_t>(sbc_tests[i].op2) << " - "
                             << static_cast<uint32_t>(sbc_tests[i].carry_in) << " = "
                             << static_cast<uint32_t>(sbc_tests[i].result)
                             << " (overflow: " << static_cast<uint32_t>(sbc_tests[i].overflow) << ", "
                             << "carry out: " << static_cast<uint32_t>(sbc_tests[i].carry_out) << ")");
        INFO("Result [" << i << "]: " << dst << " (overflow: " << state.af.flag(RegisterAF::Flags::ParityOverflow)
                        << ", "
                        << "carry out: " << state.af.flag(RegisterAF::Flags::Carry) << ")");

        bool carry_out = (sbc_tests[i].carry_out ? true : false);
        bool overflow = (sbc_tests[i].overflow ? true : false);

        REQUIRE(result == sbc_tests[i].result);
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry) == carry_out);
        REQUIRE(state.af.flag(RegisterAF::Flags::ParityOverflow) == overflow);
    }
}

TEST_CASE("Carry Overflow Sub 16bit", "[sbc]") {
    Bus mem(65536);
    Z80 state(mem, true);

    typedef struct test_data {
        uint16_t op1;
        uint16_t op2;
        uint16_t result;
        uint8_t carry_out;
        uint8_t overflow;
    } test_data;

    test_data sbc_tests[] = {
        {0, 0, 0, 0, 0},
        {0, 1, 65535, 1, 0},
        {0, 32767, 32769, 1, 0},
        {0, 32768, 32768, 1, 1},
        {0, 32769, 32767, 1, 0},
        {0, 65535, 1, 1, 0},
        {1, 0, 1, 0, 0},
        {1, 1, 0, 0, 0},
        {1, 32767, 32770, 1, 0},
        {1, 32768, 32769, 1, 1},
        {1, 32769, 32768, 1, 1},
        {1, 65535, 2, 1, 0},
        {32767, 0, 32767, 0, 0},
        {32767, 1, 32766, 0, 0},
        {32767, 32767, 0, 0, 0},
        {32767, 32768, 65535, 1, 1},
        {32767, 32769, 65534, 1, 1},
        {32767, 65535, 32768, 1, 1},
        {32768, 0, 32768, 0, 0},
        {32768, 1, 32767, 0, 1},
        {32768, 32767, 1, 0, 1},
        {32768, 32768, 0, 0, 0},
        {32768, 32769, 65535, 1, 0},
        {32768, 65535, 32769, 1, 0},
        {32769, 0, 32769, 0, 0},
        {32769, 1, 32768, 0, 0},
        {32769, 32767, 2, 0, 1},
        {32769, 32768, 1, 0, 0},
        {32769, 32769, 0, 0, 0},
        {32769, 65535, 32770, 1, 0},
        {65535, 0, 65535, 0, 0},
        {65535, 1, 65534, 0, 0},
        {65535, 32767, 32768, 0, 0},
        {65535, 32768, 32767, 0, 0},
        {65535, 32769, 32766, 0, 0},
        {65535, 65535, 0, 0, 0},
        {16383, 65535, 16384, 1, 0},
    };

    size_t length = sizeof(sbc_tests) / sizeof(sbc_tests[0]);
    for (size_t i = 0; i < length; i++) {
        state.af.flags(0);
        uint8_t dst_data[2] = {static_cast<uint8_t>(sbc_tests[i].op1 & 0xff),
                               static_cast<uint8_t>((sbc_tests[i].op1 >> 8) & 0xff)};
        StorageElement dst = StorageElement(dst_data, 2);
        StorageElement src = StorageElement(static_cast<uint8_t>(sbc_tests[i].op2 & 0xff),
                                            static_cast<uint8_t>((sbc_tests[i].op2 >> 8) & 0xff));

        Instruction instruction = Instruction(InstType::SUB, "test", 0, 0);
        instruction.do_sub(state, dst, src);

        INFO("Calculating [" << i << "]: " << static_cast<uint32_t>(sbc_tests[i].op1) << " - "
                             << static_cast<uint32_t>(sbc_tests[i].op2) << " = "
                             << static_cast<uint32_t>(sbc_tests[i].result)
                             << " (overflow: " << static_cast<uint32_t>(sbc_tests[i].overflow) << ", "
                             << "carry out: " << static_cast<uint32_t>(sbc_tests[i].carry_out) << ")");
        INFO("Result [" << i << "]: " << dst << " (overflow: " << state.af.flag(RegisterAF::Flags::ParityOverflow)
                        << ", "
                        << "carry out: " << state.af.flag(RegisterAF::Flags::Carry) << ")");

        bool carry_out = (sbc_tests[i].carry_out ? true : false);
        bool overflow = (sbc_tests[i].overflow ? true : false);
        uint32_t result = 0;
        dst.get_value(result);

        REQUIRE(result == sbc_tests[i].result);
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry) == carry_out);
        REQUIRE(state.af.flag(RegisterAF::Flags::ParityOverflow) == overflow);
    }
}
