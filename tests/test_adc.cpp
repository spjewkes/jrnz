#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
#include "z80.hpp"

TEST_CASE("Carry Overflow Add", "[adc]") {
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

    test_data adc_tests[] = {
        {0, 0, 0, 0, 0, 0},       {0, 1, 0, 1, 0, 0},       {0, 127, 0, 127, 0, 0},   {0, 128, 0, 128, 0, 0},
        {0, 129, 0, 129, 0, 0},   {0, 255, 0, 255, 0, 0},   {1, 0, 0, 1, 0, 0},       {1, 1, 0, 2, 0, 0},
        {1, 127, 0, 128, 0, 1},   {1, 128, 0, 129, 0, 0},   {1, 129, 0, 130, 0, 0},   {1, 255, 0, 0, 1, 0},
        {127, 0, 0, 127, 0, 0},   {127, 1, 0, 128, 0, 1},   {127, 127, 0, 254, 0, 1}, {127, 128, 0, 255, 0, 0},
        {127, 129, 0, 0, 1, 0},   {127, 255, 0, 126, 1, 0}, {128, 0, 0, 128, 0, 0},   {128, 1, 0, 129, 0, 0},
        {128, 127, 0, 255, 0, 0}, {128, 128, 0, 0, 1, 1},   {128, 129, 0, 1, 1, 1},   {128, 255, 0, 127, 1, 1},
        {129, 0, 0, 129, 0, 0},   {129, 1, 0, 130, 0, 0},   {129, 127, 0, 0, 1, 0},   {129, 128, 0, 1, 1, 1},
        {129, 129, 0, 2, 1, 1},   {129, 255, 0, 128, 1, 0}, {255, 0, 0, 255, 0, 0},   {255, 1, 0, 0, 1, 0},
        {255, 127, 0, 126, 1, 0}, {255, 128, 0, 127, 1, 1}, {255, 129, 0, 128, 1, 0}, {255, 255, 0, 254, 1, 0},
        {0, 0, 1, 1, 0, 0},       {0, 1, 1, 2, 0, 0},       {0, 127, 1, 128, 0, 1},   {0, 128, 1, 129, 0, 0},
        {0, 129, 1, 130, 0, 0},   {0, 255, 1, 0, 1, 0},     {1, 0, 1, 2, 0, 0},       {1, 1, 1, 3, 0, 0},
        {1, 127, 1, 129, 0, 1},   {1, 128, 1, 130, 0, 0},   {1, 129, 1, 131, 0, 0},   {1, 255, 1, 1, 1, 0},
        {127, 0, 1, 128, 0, 1},   {127, 1, 1, 129, 0, 1},   {127, 127, 1, 255, 0, 1}, {127, 128, 1, 0, 1, 0},
        {127, 129, 1, 1, 1, 0},   {127, 255, 1, 127, 1, 0}, {128, 0, 1, 129, 0, 0},   {128, 1, 1, 130, 0, 0},
        {128, 127, 1, 0, 1, 0},   {128, 128, 1, 1, 1, 1},   {128, 129, 1, 2, 1, 1},   {128, 255, 1, 128, 1, 0},
        {129, 0, 1, 130, 0, 0},   {129, 1, 1, 131, 0, 0},   {129, 127, 1, 1, 1, 0},   {129, 128, 1, 2, 1, 1},
        {129, 129, 1, 3, 1, 1},   {129, 255, 1, 129, 1, 0}, {255, 0, 1, 0, 1, 0},     {255, 1, 1, 1, 1, 0},
        {255, 127, 1, 127, 1, 0}, {255, 128, 1, 128, 1, 0}, {255, 129, 1, 129, 1, 0}, {255, 255, 1, 255, 1, 0},
    };

    size_t length = sizeof(adc_tests) / sizeof(adc_tests[0]);
    for (size_t i = 0; i < length; i++) {
        state.af.flags(0);
        uint8_t result = adc_tests[i].op1;
        StorageElement dst = StorageElement(&result, 1);
        StorageElement src = StorageElement(adc_tests[i].op2);

        state.af.flag(RegisterAF::Flags::Carry, (adc_tests[i].carry_in ? true : false));

        Instruction instruction = Instruction(InstType::ADC, "test", 0, 0);
        instruction.do_adc(state, dst, src);

        INFO("Calculating [" << i << "]: " << static_cast<uint32_t>(adc_tests[i].op1) << " + "
                             << static_cast<uint32_t>(adc_tests[i].op2) << " + "
                             << static_cast<uint32_t>(adc_tests[i].carry_in) << " = "
                             << static_cast<uint32_t>(adc_tests[i].result)
                             << " (overflow: " << static_cast<uint32_t>(adc_tests[i].overflow) << ", "
                             << "carry out: " << static_cast<uint32_t>(adc_tests[i].carry_out) << ")");
        INFO("Result [" << i << "]: " << dst << " (overflow: " << state.af.flag(RegisterAF::Flags::ParityOverflow)
                        << ", "
                        << "carry out: " << state.af.flag(RegisterAF::Flags::Carry) << ")");

        bool carry_out = (adc_tests[i].carry_out ? true : false);
        bool overflow = (adc_tests[i].overflow ? true : false);

        REQUIRE(result == adc_tests[i].result);
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry) == carry_out);
        REQUIRE(state.af.flag(RegisterAF::Flags::ParityOverflow) == overflow);
    }
}

TEST_CASE("Carry Overflow Add 16bit", "[adc]") {
    Bus mem(65536);
    Z80 state(mem, true);

    typedef struct test_data {
        uint16_t op1;
        uint16_t op2;
        uint16_t result;
        uint8_t carry_out;
        uint8_t overflow;
    } test_data;

    test_data adc_tests[] = {
        {0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 32767, 32767, 0, 0},
        {0, 32768, 32768, 0, 0},
        {0, 32769, 32769, 0, 0},
        {0, 65535, 65535, 0, 0},
        {1, 0, 1, 0, 0},
        {1, 1, 2, 0, 0},
        {1, 32767, 32768, 0, 1},
        {1, 32768, 32769, 0, 0},
        {1, 32769, 32770, 0, 0},
        {1, 65535, 0, 1, 0},
        {32767, 0, 32767, 0, 0},
        {32767, 1, 32768, 0, 1},
        {32767, 32767, 65534, 0, 1},
        {32767, 32768, 65535, 0, 0},
        {32767, 32769, 0, 1, 0},
        {32767, 65535, 32766, 1, 0},
        {32768, 0, 32768, 0, 0},
        {32768, 1, 32769, 0, 0},
        {32768, 32767, 65535, 0, 0},
        {32768, 32768, 0, 1, 1},
        {32768, 32769, 1, 1, 1},
        {32768, 65535, 32767, 1, 1},
        {32769, 0, 32769, 0, 0},
        {32769, 1, 32770, 0, 0},
        {32769, 32767, 0, 1, 0},
        {32769, 32768, 1, 1, 1},
        {32769, 32769, 2, 1, 1},
        {32769, 65535, 32768, 1, 0},
        {65535, 0, 65535, 0, 0},
        {65535, 1, 0, 1, 0},
        {65535, 32767, 32766, 1, 0},
        {65535, 32768, 32767, 1, 1},
        {65535, 32769, 32768, 1, 0},
        {65535, 65535, 65534, 1, 0},
    };

    size_t length = sizeof(adc_tests) / sizeof(adc_tests[0]);
    for (size_t i = 0; i < length; i++) {
        state.af.flags(0);
        uint8_t dst_data[2] = {static_cast<uint8_t>(adc_tests[i].op1 & 0xff),
                               static_cast<uint8_t>((adc_tests[i].op1 >> 8) & 0xff)};
        StorageElement dst = StorageElement(dst_data, 2);
        StorageElement src = StorageElement(static_cast<uint8_t>(adc_tests[i].op2 & 0xff),
                                            static_cast<uint8_t>((adc_tests[i].op2 >> 8) & 0xff));

        Instruction instruction = Instruction(InstType::ADC, "test", 0, 0);
        instruction.do_adc(state, dst, src);

        INFO("Calculating [" << i << "]: " << static_cast<uint32_t>(adc_tests[i].op1) << " + "
                             << static_cast<uint32_t>(adc_tests[i].op2) << " = "
                             << static_cast<uint32_t>(adc_tests[i].result)
                             << " (overflow: " << static_cast<uint32_t>(adc_tests[i].overflow) << ", "
                             << "carry out: " << static_cast<uint32_t>(adc_tests[i].carry_out) << ")");
        INFO("Result [" << i << "]: " << dst << " (overflow: " << state.af.flag(RegisterAF::Flags::ParityOverflow)
                        << ", "
                        << "carry out: " << state.af.flag(RegisterAF::Flags::Carry) << ")");

        bool carry_out = (adc_tests[i].carry_out ? true : false);
        bool overflow = (adc_tests[i].overflow ? true : false);
        uint32_t result = 0;
        dst.get_value(result);

        REQUIRE(result == adc_tests[i].result);
        REQUIRE(state.af.flag(RegisterAF::Flags::Carry) == carry_out);
        REQUIRE(state.af.flag(RegisterAF::Flags::ParityOverflow) == overflow);
    }
}
