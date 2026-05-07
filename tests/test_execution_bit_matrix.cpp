#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "test_support.hpp"

namespace {
uint8_t expected_bit_flags(uint8_t value, uint8_t bit_index, bool carry_in, uint8_t f3_f5_source) {
    const bool bit_set = (value & static_cast<uint8_t>(1u << bit_index)) != 0;
    uint8_t flags = carry_in ? 0x01 : 0x00;
    flags |= 0x10;  // H
    if (!bit_set) {
        flags |= 0x40;  // Z
        flags |= 0x04;  // P/V mirrors Z
    }
    if (bit_index == 7 && bit_set) {
        flags |= 0x80;
    }
    flags |= f3_f5_source & 0x28;
    return flags;
}
}  // namespace

TEST_CASE("BIT register and HL forms match documented and undocumented flag formulas",
          "[bitsetres][compliance][bit-matrix]") {
    struct RegisterCase {
        const char *name;
        uint8_t prefix_opcode;
        uint8_t *reg_ptr;
    };

    SECTION("Register BIT forms") {
        for (uint8_t bit = 0; bit < 8; ++bit) {
            for (uint8_t reg = 0; reg < 8; ++reg) {
                if (reg == 6) {
                    continue;
                }
                for (unsigned int value_raw = 0; value_raw <= 0xff; ++value_raw) {
                    const uint8_t value = static_cast<uint8_t>(value_raw);
                    for (bool carry_in : {false, true}) {
                        CpuHarness h;
                        const uint8_t opcode = static_cast<uint8_t>(0x40u | (bit << 3) | reg);
                        INFO("bit=" << static_cast<unsigned int>(bit) << " reg=" << static_cast<unsigned int>(reg)
                                    << " value=0x" << std::hex << static_cast<unsigned int>(value)
                                    << " carry_in=" << carry_in);

                        switch (reg) {
                            case 0:
                                h.cpu.bc.hi(value);
                                break;
                            case 1:
                                h.cpu.bc.lo(value);
                                break;
                            case 2:
                                h.cpu.de.hi(value);
                                break;
                            case 3:
                                h.cpu.de.lo(value);
                                break;
                            case 4:
                                h.cpu.hl.hi(value);
                                break;
                            case 5:
                                h.cpu.hl.lo(value);
                                break;
                            case 7:
                                h.cpu.af.accum(value);
                                break;
                            default:
                                break;
                        }

                        h.cpu.af.flags(0x00);
                        h.cpu.af.flag(RegisterAF::Flags::Carry, carry_in);
                        h.load({0xcb, opcode});

                        const StepResult step = h.step();

                        REQUIRE(step.cycle_delta() == 8);
                        REQUIRE(step.pc_after == 0x0002);
                        require_flags(h.cpu.af.flags(), expected_bit_flags(value, bit, carry_in, value));
                    }
                }
            }
        }
    }

    SECTION("BIT (HL) forms") {
        for (uint8_t bit = 0; bit < 8; ++bit) {
            for (unsigned int value_raw = 0; value_raw <= 0xff; ++value_raw) {
                const uint8_t value = static_cast<uint8_t>(value_raw);
                for (bool carry_in : {false, true}) {
                    CpuHarness h;
                    const uint8_t opcode = static_cast<uint8_t>(0x46u | (bit << 3));
                    INFO("bit=" << static_cast<unsigned int>(bit) << " value=0x" << std::hex
                                << static_cast<unsigned int>(value) << " carry_in=" << carry_in);

                    h.cpu.hl.set(0x9800);
                    h.cpu.memptr.set(0x2800);
                    h.mem[0x9800] = value;
                    h.cpu.af.flags(0x00);
                    h.cpu.af.flag(RegisterAF::Flags::Carry, carry_in);
                    h.load({0xcb, opcode});

                    const StepResult step = h.step();

                    REQUIRE(step.cycle_delta() == 12);
                    REQUIRE(step.pc_after == 0x0002);
                    REQUIRE(h.mem[0x9800] == value);
                    require_flags(h.cpu.af.flags(), expected_bit_flags(value, bit, carry_in, 0x28));
                }
            }
        }
    }
}
