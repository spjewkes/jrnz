#include <array>
#include <catch2/catch_test_macros.hpp>

#include "ay.hpp"
#include "bus.hpp"

namespace {
void write_ay_register(Bus &bus, uint8_t reg, uint8_t value) {
    bus.write_port(0xfffd, reg);
    bus.write_port(0xbffd, value);
}
}  // namespace

TEST_CASE("AY register writes are masked to the chip-visible bit widths", "[ay]") {
    AyChip ay(spectrum_128k_model());

    ay.select_register(1);
    ay.write_selected_register(0xff);
    REQUIRE(ay.register_value(1) == 0x0f);

    ay.select_register(6);
    ay.write_selected_register(0xff);
    REQUIRE(ay.register_value(6) == 0x1f);

    ay.select_register(8);
    ay.write_selected_register(0xff);
    REQUIRE(ay.register_value(8) == 0x1f);

    ay.select_register(14);
    ay.write_selected_register(0xff);
    REQUIRE(ay.register_value(14) == 0xff);
}

TEST_CASE("AY tone channel toggles after the programmed half-period", "[ay]") {
    AyChip ay(spectrum_128k_model());
    ay.select_register(0);
    ay.write_selected_register(0x01);
    ay.select_register(1);
    ay.write_selected_register(0x00);
    ay.select_register(7);
    ay.write_selected_register(0x3e);
    ay.select_register(8);
    ay.write_selected_register(0x0f);

    const int32_t initial_level = ay.output_level();
    REQUIRE(initial_level > 0);

    for (uint8_t i = 0; i < 17; ++i) {
        ay.clock_cpu_tstate();
    }

    REQUIRE(ay.output_level() < 0);
}

TEST_CASE("128K bus AY registers can drive the AY sound model", "[ay][bus]") {
    Bus bus(spectrum_128k_model());

    write_ay_register(bus, 0, 0x01);
    write_ay_register(bus, 1, 0x00);
    write_ay_register(bus, 7, 0x3e);
    write_ay_register(bus, 8, 0x0f);

    AyChip ay(bus.model());
    for (uint8_t reg = 0; reg < 16; ++reg) {
        ay.write_register(reg, bus.ay_register(reg));
    }

    const int32_t initial_level = ay.output_level();
    REQUIRE(initial_level > 0);

    for (uint8_t i = 0; i < 17; ++i) {
        ay.clock_cpu_tstate();
    }

    REQUIRE(ay.output_level() < 0);
}

TEST_CASE("48K AY model stays silent", "[ay]") {
    AyChip ay(spectrum_48k_model());
    ay.select_register(8);
    ay.write_selected_register(0x0f);

    for (uint8_t i = 0; i < 16; ++i) {
        ay.clock_cpu_tstate();
    }

    REQUIRE(ay.output_level() == 0);
}
