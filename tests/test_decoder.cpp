#include <catch2/catch_test_macros.hpp>

#include "decoder.hpp"

TEST_CASE("Base opcode table covers all non-prefix opcodes", "[decoder]") {
    for (uint32_t opcode = 0x00; opcode <= 0xff; ++opcode) {
        const Instruction &inst = decode_opcode(opcode);
        const bool is_prefix = opcode == 0xcb || opcode == 0xdd || opcode == 0xed || opcode == 0xfd;

        INFO("opcode=0x" << std::hex << opcode);
        if (is_prefix) {
            REQUIRE(inst.inst == InstType::INV);
        } else {
            REQUIRE(inst.inst != InstType::INV);
            REQUIRE(inst.size >= 1);
        }
    }
}

TEST_CASE("CB prefixed opcode table is complete", "[decoder]") {
    for (uint32_t opcode = 0x00; opcode <= 0xff; ++opcode) {
        const Instruction &inst = decode_opcode(0xcb00 | opcode);
        INFO("opcode=0xcb" << std::hex << opcode);
        REQUIRE(inst.inst != InstType::INV);
        REQUIRE(inst.size == 2);
    }
}

TEST_CASE("ED prefixed opcode table keeps documented coverage stable", "[decoder]") {
    size_t valid_count = 0;
    for (uint32_t opcode = 0x00; opcode <= 0xff; ++opcode) {
        const Instruction &inst = decode_opcode(0xed00 | opcode);
        if (inst.inst != InstType::INV) {
            ++valid_count;
            REQUIRE((inst.size == 2 || inst.size == 4));
        }
    }

    REQUIRE(valid_count == 74);
    REQUIRE(decode_opcode(0xed44).inst == InstType::NEG);
    REQUIRE(decode_opcode(0xed4d).inst == InstType::RETI);
    REQUIRE(decode_opcode(0xed57).inst == InstType::LD);
    REQUIRE(decode_opcode(0xeda0).inst == InstType::LDI);
    REQUIRE(decode_opcode(0xed04).inst == InstType::INV);
}

TEST_CASE("Indexed prefix decoding handles both overrides and ignored prefixes", "[decoder]") {
    const Instruction &dd_specific = decode_opcode(0xdd09);
    REQUIRE(dd_specific.inst == InstType::ADD);
    REQUIRE(dd_specific.dst == Operand::IX);
    REQUIRE(dd_specific.src == Operand::BC);
    REQUIRE(dd_specific.size == 2);

    const Instruction &fd_specific = decode_opcode(0xfd21);
    REQUIRE(fd_specific.inst == InstType::LD);
    REQUIRE(fd_specific.dst == Operand::IY);
    REQUIRE(fd_specific.src == Operand::NN);
    REQUIRE(fd_specific.size == 4);

    const Instruction &dd_fallback = decode_opcode(0xdd40);
    REQUIRE(dd_fallback.inst == InstType::LD);
    REQUIRE(dd_fallback.dst == Operand::B);
    REQUIRE(dd_fallback.src == Operand::B);
    REQUIRE(dd_fallback.size == 2);

    const Instruction &fd_fallback = decode_opcode(0xfdc9);
    REQUIRE(fd_fallback.inst == InstType::RET);
    REQUIRE(fd_fallback.cond == Conditional::ALWAYS);
    REQUIRE(fd_fallback.size == 2);

    REQUIRE(decode_opcode(0xddcb06).inst == InstType::RLC);
    REQUIRE(decode_opcode(0xfdcb7e).inst == InstType::BIT);
}

TEST_CASE("Indexed bit-op tables remain complete", "[decoder]") {
    size_t ddcb_count = 0;
    size_t fdcb_count = 0;
    for (uint32_t opcode = 0x00; opcode <= 0xff; ++opcode) {
        if (decode_opcode(0xddcb00 | opcode).inst != InstType::INV) {
            ++ddcb_count;
        }
        if (decode_opcode(0xfdcb00 | opcode).inst != InstType::INV) {
            ++fdcb_count;
        }
    }

    REQUIRE(ddcb_count == 32);
    REQUIRE(fdcb_count == 32);
}
