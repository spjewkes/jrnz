#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"

TEST_CASE("Bus preserves ROM write protection and RAM visibility", "[bus]") {
    Bus bus(65536);

    bus[0x3fff] = 0x11;
    bus[0x4000] = 0x22;

    bus.write_data(0x3fff, 0xaa);
    bus.write_data(0x4000, 0xbb);

    REQUIRE(bus.read_data(0x3fff) == 0x11);
    REQUIRE(bus.read_data(0x4000) == 0xbb);
}

TEST_CASE("Word writes respect the ROM to RAM boundary one byte at a time", "[bus]") {
    Bus bus(65536);

    bus[0x3fff] = 0x12;
    bus[0x4000] = 0x34;

    bus.write_addr_to_mem(0x3fff, 0xabcd);

    REQUIRE(bus.read_data(0x3fff) == 0x12);
    REQUIRE(bus.read_data(0x4000) == 0xab);
}

TEST_CASE("Port reads distinguish even keyboard ports from the floating bus path", "[bus]") {
    Bus bus(65536);
    bus[0x4000] = 0x81;
    bus[0x4001] = 0x42;
    bus.floating_counter = 0;

    const uint8_t even = bus.read_port(0x00fe);
    REQUIRE(even == 0xff);
    REQUIRE(bus.floating_counter == 0);

    const uint8_t first_odd = bus.read_port(0x00ff);
    const uint8_t second_odd = bus.read_port(0x00ff);
    REQUIRE(first_odd == 0x81);
    REQUIRE(second_odd == 0x42);
    REQUIRE(bus.floating_counter == 2);
}

TEST_CASE("Floating bus reads advance through the 16K display region and wrap around", "[bus]") {
    Bus bus(65536);
    bus[0x4000] = 0x11;
    bus[0x4001] = 0x22;
    bus[0x7fff] = 0x33;

    bus.floating_counter = 0x3fff;
    REQUIRE(bus.read_port(0x00ff) == 0x33);
    REQUIRE(bus.floating_counter == 0x4000);

    REQUIRE(bus.read_port(0x00ff) == 0x11);
    REQUIRE(bus.floating_counter == 0x4001);

    REQUIRE(bus.read_port(0x00ff) == 0x22);
    REQUIRE(bus.floating_counter == 0x4002);
}

TEST_CASE("Even keyboard-port reads preserve the upper three bits regardless of row selection", "[bus]") {
    Bus bus(65536);

    const uint8_t all_rows = bus.read_port(0x00fe);
    const uint8_t middle_rows = bus.read_port(0xbffe);
    const uint8_t high_rows = bus.read_port(0x7ffe);

    REQUIRE((all_rows & 0xe0) == 0xe0);
    REQUIRE((middle_rows & 0xe0) == 0xe0);
    REQUIRE((high_rows & 0xe0) == 0xe0);
}

TEST_CASE("ROM writes remain blocked even when addressing through read-modify-write helpers", "[bus]") {
    Bus bus(65536);
    bus[0x3ffe] = 0x12;
    bus[0x3fff] = 0x34;
    bus[0x4000] = 0x56;

    bus.write_addr_to_mem(0x3ffe, 0xabcd);

    REQUIRE(bus.read_data(0x3ffe) == 0x12);
    REQUIRE(bus.read_data(0x3fff) == 0x34);
    REQUIRE(bus.read_data(0x4000) == 0x56);
}

TEST_CASE("Port writes only latch the Spectrum ULA port when the low byte is 0xfe", "[bus]") {
    Bus bus(65536);

    bus.port_254 = 0x00;
    bus.write_port(0x12fe, 0x77);
    REQUIRE(bus.port_254 == 0x77);

    bus.write_port(0x12ff, 0x33);
    REQUIRE(bus.port_254 == 0x77);
}

TEST_CASE("Contention adds wait states for accesses in contended RAM during the active display window", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_data(bus.model().contention_ram_base);

    REQUIRE(bus.end_instruction_timing() == 6);
}

TEST_CASE("Contention does not add wait states outside contended RAM or outside the display window", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(0);
    bus.begin_instruction_timing();
    (void)bus.read_data(bus.model().contention_ram_base);
    REQUIRE(bus.end_instruction_timing() == 0);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base - 1));
    REQUIRE(bus.end_instruction_timing() == 0);
}
