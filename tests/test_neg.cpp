#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
#include "test_support.hpp"
#include "z80.hpp"

TEST_CASE("Negative", "[neg]") {
    Bus mem(65536);
    Z80 state(mem, true);
    Instruction instruction = Instruction(InstType::NEG, "test", 0, 0, Operand::A, Operand::A);

    typedef struct test_data {
        uint8_t op1;
        uint8_t result;
        uint8_t flags;
    } test_data;

    test_data neg_tests[] = {
        {0x00, 0x00, 0x42}, {0x01, 0xff, 0xbb}, {0x08, 0xf8, 0xbb}, {0x28, 0xd8, 0x9b}, {0x80, 0x80, 0x87},
    };

    size_t length = sizeof(neg_tests) / sizeof(neg_tests[0]);
    for (size_t i = 0; i < length; i++) {
        state.af.flags(0);
        uint8_t result = neg_tests[i].op1;
        StorageElement dst = StorageElement(&result, 1);
        StorageElement src = StorageElement(neg_tests[i].op1);

        instruction.do_neg(state, dst, src);

        INFO("Calculating [" << i << "]: NEG " << static_cast<uint32_t>(neg_tests[i].op1) << " = "
                             << static_cast<uint32_t>(neg_tests[i].result));
        INFO("Result [" << i << "]: " << dst);

        REQUIRE(result == neg_tests[i].result);
        require_flags(state.af.flags(), neg_tests[i].flags);
    }
}
