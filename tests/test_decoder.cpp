#include <array>
#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"
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

TEST_CASE("ED prefixed opcode table separates documented coverage from undocumented aliases", "[decoder]") {
    constexpr std::array<uint32_t, 18> undocumented_ed_opcodes = {
        0xed4c, 0xed4e, 0xed54, 0xed55, 0xed5c, 0xed5d, 0xed64, 0xed65, 0xed66,
        0xed6c, 0xed6d, 0xed6e, 0xed71, 0xed74, 0xed75, 0xed76, 0xed7c, 0xed7d,
    };

    size_t documented_count = 0;
    size_t undocumented_count = 0;
    size_t undefined_nop_count = 0;
    for (uint32_t opcode = 0x00; opcode <= 0xff; ++opcode) {
        const uint32_t full_opcode = 0xed00 | opcode;
        const Instruction &inst = decode_opcode(full_opcode);
        if (inst.inst != InstType::INV) {
            REQUIRE((inst.size == 2 || inst.size == 4));
            const bool is_undocumented = std::find(undocumented_ed_opcodes.begin(), undocumented_ed_opcodes.end(),
                                                   full_opcode) != undocumented_ed_opcodes.end();
            if (is_undocumented) {
                ++undocumented_count;
            } else if (inst.inst == InstType::NOP && inst.size == 2 && inst.cycles == 8) {
                ++undefined_nop_count;
            } else {
                ++documented_count;
            }
        }
    }

    REQUIRE(documented_count == 58);
    REQUIRE(undocumented_count == undocumented_ed_opcodes.size());
    REQUIRE(undefined_nop_count == 180);
    REQUIRE(decode_opcode(0xed44).inst == InstType::NEG);
    REQUIRE(decode_opcode(0xed4d).inst == InstType::RETI);
    REQUIRE(decode_opcode(0xed57).inst == InstType::LD);
    REQUIRE(decode_opcode(0xeda0).inst == InstType::LDI);
    REQUIRE(decode_opcode(0xed4e).inst == InstType::IM);
    REQUIRE(decode_opcode(0xed71).inst == InstType::OUT);
    REQUIRE(decode_opcode(0xed04).inst == InstType::NOP);
    REQUIRE(decode_opcode(0xed04).size == 2);
    REQUIRE(decode_opcode(0xed04).cycles == 8);
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

TEST_CASE("Opcode fetch preserves prefix semantics and operand offsets", "[decoder][fetch]") {
    auto load = [](Bus &bus, std::initializer_list<uint8_t> bytes, uint16_t start = 0x0000) {
        size_t offset = 0;
        for (uint8_t byte : bytes) {
            bus[static_cast<uint16_t>(start + offset)] = byte;
            ++offset;
        }
    };

    SECTION("Repeated DD prefixes count as ignored prefixes before the final indexed opcode") {
        Bus bus(65536);
        load(bus, {0xdd, 0xdd, 0x21, 0x34, 0x12});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xdd21);
        REQUIRE(fetched.ignored_prefixes == 1);
        REQUIRE(fetched.fetch_len == 3);
        REQUIRE(fetched.operand_offset == 3);
    }

    SECTION("Mixed DD and FD chains leave only the last indexed prefix in effect") {
        Bus bus(65536);
        load(bus, {0xdd, 0xfd, 0x77, 0x05});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xfd77);
        REQUIRE(fetched.ignored_prefixes == 1);
        REQUIRE(fetched.fetch_len == 3);
        REQUIRE(fetched.operand_offset == 3);
    }

    SECTION("DD before ED is ignored and leaves an ED opcode") {
        Bus bus(65536);
        load(bus, {0xdd, 0xed, 0x44});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xed44);
        REQUIRE(fetched.ignored_prefixes == 1);
        REQUIRE(fetched.fetch_len == 3);
        REQUIRE(fetched.operand_offset == 3);
    }

    SECTION("FD before ED is ignored and leaves an ED opcode") {
        Bus bus(65536);
        load(bus, {0xfd, 0xed, 0x4d});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xed4d);
        REQUIRE(fetched.ignored_prefixes == 1);
        REQUIRE(fetched.fetch_len == 3);
        REQUIRE(fetched.operand_offset == 3);
    }

    SECTION("DDCB forms keep the displacement as an operand rather than part of fetch length") {
        Bus bus(65536);
        load(bus, {0xdd, 0xcb, 0xfe, 0x46});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xddcb46);
        REQUIRE(fetched.ignored_prefixes == 0);
        REQUIRE(fetched.fetch_len == 3);
        REQUIRE(fetched.operand_offset == 2);
    }

    SECTION("Ignored prefixes ahead of DDCB still contribute to fetch length and operand offset") {
        Bus bus(65536);
        load(bus, {0xdd, 0xdd, 0xcb, 0x08, 0x4e});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xddcb4e);
        REQUIRE(fetched.ignored_prefixes == 1);
        REQUIRE(fetched.fetch_len == 4);
        REQUIRE(fetched.operand_offset == 3);
    }

    SECTION("FDCB forms report the displacement byte after the indexed prefix pair") {
        Bus bus(65536);
        load(bus, {0xfd, 0xcb, 0x10, 0x86});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xfdcb86);
        REQUIRE(fetched.ignored_prefixes == 0);
        REQUIRE(fetched.fetch_len == 3);
        REQUIRE(fetched.operand_offset == 2);
    }

    SECTION("Plain ED fetches report the second byte as the first operand position") {
        Bus bus(65536);
        load(bus, {0xed, 0x57});

        const FetchedOpcode fetched = bus.read_opcode_from_mem(0x0000);

        REQUIRE(fetched.opcode == 0xed57);
        REQUIRE(fetched.ignored_prefixes == 0);
        REQUIRE(fetched.fetch_len == 2);
        REQUIRE(fetched.operand_offset == 2);
    }
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

    REQUIRE(ddcb_count == 200);
    REQUIRE(fdcb_count == 200);
}
