#include "test.hpp"
#include "z80.hpp"
#include "bus.hpp"

static void test_carry_overflow()
{
	Bus mem(65536);
	Z80 state(mem, true);

	typedef struct test_data
	{
		uint8_t op1;
		uint8_t op2;
		uint8_t carry_in;
		uint8_t result;
		uint8_t carry_out;
		uint8_t overflow;
	} test_data;

	test_data adc_tests[] =
		{
			{0,   0, 0,   0, 0, 0},
			{0,   1, 0,   1, 0, 0},
			{0, 127, 0, 127, 0, 0},
			{0, 128, 0, 128, 0, 0},
			{0, 129, 0, 129, 0, 0},
			{0, 255, 0, 255, 0, 0},
			{1,   0, 0,   1, 0, 0},
			{1,   1, 0,   2, 0, 0},
			{1, 127, 0, 128, 0, 1},
			{1, 128, 0, 129, 0, 0},
			{1, 129, 0, 130, 0, 0},
			{1, 255, 0,   0, 1, 0},
			{127,   0, 0, 127, 0, 0},
			{127,   1, 0, 128, 0, 1},
			{127, 127, 0, 254, 0, 1},
			{127, 128, 0, 255, 0, 0},
			{127, 129, 0,   0, 1, 0},
			{127, 255, 0, 126, 1, 0},
			{128,   0, 0, 128, 0, 0},
			{128,   1, 0, 129, 0, 0},
			{128, 127, 0, 255, 0, 0},
			{128, 128, 0,   0, 1, 1},
			{128, 129, 0,   1, 1, 1},
			{128, 255, 0, 127, 1, 1},
			{129,   0, 0, 129, 0, 0},
			{129,   1, 0, 130, 0, 0},
			{129, 127, 0,   0, 1, 0},
			{129, 128, 0,   1, 1, 1},
			{129, 129, 0,   2, 1, 1},
			{129, 255, 0, 128, 1, 0},
			{255,   0, 0, 255, 0, 0},
			{255,   1, 0,   0, 1, 0},
			{255, 127, 0, 126, 1, 0},
			{255, 128, 0, 127, 1, 1},
			{255, 129, 0, 128, 1, 0},
			{255, 255, 0, 254, 1, 0},
			{0,   0, 1,   1, 0, 0},
			{0,   1, 1,   2, 0, 0},
			{0, 127, 1, 128, 0, 1},
			{0, 128, 1, 129, 0, 0},
			{0, 129, 1, 130, 0, 0},
			{0, 255, 1,   0, 1, 0},
			{1,   0, 1,   2, 0, 0},
			{1,   1, 1,   3, 0, 0},
			{1, 127, 1, 129, 0, 1},
			{1, 128, 1, 130, 0, 0},
			{1, 129, 1, 131, 0, 0},
			{1, 255, 1,   1, 1, 0},
			{127,   0, 1, 128, 0, 1},
			{127,   1, 1, 129, 0, 1},
			{127, 127, 1, 255, 0, 1},
			{127, 128, 1,   0, 1, 0},
			{127, 129, 1,   1, 1, 0},
			{127, 255, 1, 127, 1, 0},
			{128,   0, 1, 129, 0, 0},
			{128,   1, 1, 130, 0, 0},
			{128, 127, 1,   0, 1, 0},
			{128, 128, 1,   1, 1, 1},
			{128, 129, 1,   2, 1, 1},
			{128, 255, 1, 128, 1, 0},
			{129,   0, 1, 130, 0, 0},
			{129,   1, 1, 131, 0, 0},
			{129, 127, 1,   1, 1, 0},
			{129, 128, 1,   2, 1, 1},
			{129, 129, 1,   3, 1, 1},
			{129, 255, 1, 129, 1, 0},
			{255,   0, 1,   0, 1, 0},
			{255,   1, 1,   1, 1, 0},
			{255, 127, 1, 127, 1, 0},
			{255, 128, 1, 128, 1, 0},
			{255, 129, 1, 129, 1, 0},
			{255, 255, 1, 255, 1, 0},
		};

	size_t length = sizeof(adc_tests) / sizeof(adc_tests[0]);
	for (size_t i=0; i<length; i++)
	{
		state.af.flags(0);
		uint8_t result = adc_tests[i].op1;
		StorageElement dst = StorageElement(&result, 1);
		StorageElement src = StorageElement(adc_tests[i].op2);
		if (adc_tests[i].carry_in)
		{
			state.af.flag(RegisterAF::Flags::Carry, true);
		}

		std::cout << "Adding " << src << " to " << dst << " (and carry " << static_cast<uint32_t>(adc_tests[i].carry_in) << ") ";

		Instruction instruction = Instruction(InstType::ADC, "test", 0, 0);
		instruction.do_adc(state, dst, src);

		std::cout << "gives: " << dst << std::endl;

		bool carry_out = (adc_tests[i].carry_out ? true : false);
		bool overflow = (adc_tests[i].overflow ? true : false);

		BOOST_CHECK_EQUAL(result, adc_tests[i].result);
		BOOST_CHECK_EQUAL(state.af.flag(RegisterAF::Flags::Carry), carry_out);
		BOOST_CHECK_EQUAL(state.af.flag(RegisterAF::Flags::ParityOverflow), overflow);
	}
}

test_suite* create_test_suite_adc()
{
	test_suite* ts = BOOST_TEST_SUITE("test_suite_adc");

	ts->add(BOOST_TEST_CASE(&test_carry_overflow));

	return ts;
}
