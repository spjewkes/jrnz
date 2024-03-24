#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;
#pragma GCC diagnostic pop

#include <cstdlib>
#include <iostream>

#include "bus.hpp"
#include "z80.hpp"

/**
 * @brief Test suite create prototypes.
 */
test_suite* create_test_suite_adc();
test_suite* create_test_suite_sbc();

/**
 * @brief Main entry-point into application.
 */
test_suite* init_unit_test_suite(int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    framework::master_test_suite().add(create_test_suite_adc());
    framework::master_test_suite().add(create_test_suite_sbc());

    return EXIT_SUCCESS;
}
