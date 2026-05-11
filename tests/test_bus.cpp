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

TEST_CASE("Floating bus returns 0xff outside the active display window when beam timing is known", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(0);
    REQUIRE(bus.read_port(0x00ff) == 0xff);
}

TEST_CASE("Floating bus follows bitmap and attribute fetch phases during the display window", "[bus]") {
    Bus bus(65536);
    const uint16_t bitmap = bus.model().screen_bitmap_base;
    const uint16_t attr = bus.model().screen_attr_base;

    bus[bitmap] = 0x12;
    bus[attr] = 0x34;

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    REQUIRE(bus.read_port(0x00ff) == 0x12);

    bus.set_frame_tstate(static_cast<uint64_t>(bus.model().contention_first_tstate + 2));
    REQUIRE(bus.read_port(0x00ff) == 0x34);
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

TEST_CASE("Port 0xfe exposes the EAR input on bit 6 while keeping the fixed high bits set", "[bus]") {
    Bus bus(65536);

    bus.set_input_line(MachineInputLine::Ear, true);
    REQUIRE(bus.input_line_active(MachineInputLine::Ear));
    REQUIRE((bus.read_port(0x00fe) & 0xe0) == 0xe0);

    bus.set_input_line(MachineInputLine::Ear, false);
    REQUIRE_FALSE(bus.input_line_active(MachineInputLine::Ear));
    REQUIRE((bus.read_port(0x00fe) & 0xe0) == 0xa0);
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

TEST_CASE("Contention follows the 8-tstate ULA delay pattern across successive accesses", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    for (int i = 0; i < 8; ++i) {
        (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base + i));
    }

    REQUIRE(bus.end_instruction_timing() == 21);
}

TEST_CASE("Contention uses the frame phase of each successive memory access within one instruction", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(static_cast<uint64_t>(bus.model().contention_first_tstate + 6));
    bus.begin_instruction_timing();
    (void)bus.read_data(bus.model().contention_ram_base);
    (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base + 1));
    (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base + 2));

    REQUIRE(bus.end_instruction_timing() == 6);
}

TEST_CASE("Opcode fetch and operand reads both contribute to contention timing", "[bus]") {
    Bus bus(65536);

    const uint16_t base = bus.model().contention_ram_base;
    bus[base] = 0xdd;
    bus[static_cast<uint16_t>(base + 1)] = 0xcb;
    bus[static_cast<uint16_t>(base + 2)] = 0x05;
    bus[static_cast<uint16_t>(base + 3)] = 0x46;

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    const FetchedOpcode fetched = bus.read_opcode_from_mem(base);

    REQUIRE(fetched.opcode == 0xddcb46);
    REQUIRE(fetched.fetch_len == 3);
    REQUIRE(bus.end_instruction_timing() == 15);
}

TEST_CASE("Odd ports outside the contended address range do not add I/O wait states", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_port(0x00ff);

    REQUIRE(bus.end_instruction_timing() == 0);
}

TEST_CASE("Even ULA ports add the expected I/O contention pattern", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_port(0x00fe);

    REQUIRE(bus.end_instruction_timing() == 5);
}

TEST_CASE("Ports with a contended high byte incur full four-cycle I/O contention", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_port(0x40ff);
    REQUIRE(bus.end_instruction_timing() == 18);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    bus.write_port(0x40fe, 0x03);
    REQUIRE(bus.end_instruction_timing() == 11);
}
