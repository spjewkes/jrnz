#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
#include "z80.hpp"

TEST_CASE("Negative", "[neg]") {
    Bus mem(65536);
    Z80 state(mem, true);

    typedef struct test_data {
        uint8_t op1;
        uint8_t result;
    } test_data;

    test_data neg_tests[] = {
        {0x28, 0xD8},
    };

    size_t length = sizeof(neg_tests) / sizeof(neg_tests[0]);
    for (size_t i = 0; i < length; i++) {
        state.af.flags(0);
        uint8_t result = neg_tests[i].op1;
        StorageElement dst = StorageElement(&result, 1);
        StorageElement src = StorageElement(neg_tests[i].op1);

        Instruction instruction = Instruction(InstType::NEG, "test", 0, 0, Operand::A, Operand::A);
        instruction.do_neg(state, dst, src);

        INFO("Calculating [" << i << "]: NEG " << static_cast<uint32_t>(neg_tests[i].op1) << " = "
                             << static_cast<uint32_t>(neg_tests[i].result));
        INFO("Result [" << i << "]: " << dst);

        REQUIRE(result == neg_tests[i].result);
    }
}
