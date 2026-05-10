#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "test_support.hpp"

namespace {
enum class ImmOp {
    Add,
    Adc,
    Sub,
    Sbc,
    And,
    Xor,
    Or,
    Cp,
};

uint8_t parity_flag(uint8_t value) {
    return (static_cast<uint8_t>(__builtin_popcount(static_cast<unsigned int>(value)) % 2) == 0) ? 0x04 : 0x00;
}

uint8_t add_flags(uint8_t a, uint8_t operand, bool carry_in, uint8_t result) {
    const uint16_t full = static_cast<uint16_t>(a) + static_cast<uint16_t>(operand) + (carry_in ? 1u : 0u);
    uint8_t flags = 0;
    if ((result & 0x80) != 0) {
        flags |= 0x80;
    }
    if (result == 0) {
        flags |= 0x40;
    }
    if ((((a & 0x0f) + (operand & 0x0f) + (carry_in ? 1u : 0u)) & 0x10) != 0) {
        flags |= 0x10;
    }
    if ((~(a ^ operand) & (a ^ result) & 0x80) != 0) {
        flags |= 0x04;
    }
    if ((result & 0x08) != 0) {
        flags |= 0x08;
    }
    if ((result & 0x20) != 0) {
        flags |= 0x20;
    }
    if (full > 0xff) {
        flags |= 0x01;
    }
    return flags;
}

uint8_t sub_flags(uint8_t a, uint8_t operand, bool carry_in, uint8_t result) {
    const uint16_t subtrahend = static_cast<uint16_t>(operand) + (carry_in ? 1u : 0u);
    uint8_t flags = 0x02;
    if ((result & 0x80) != 0) {
        flags |= 0x80;
    }
    if (result == 0) {
        flags |= 0x40;
    }
    if ((a & 0x0f) < ((operand & 0x0f) + (carry_in ? 1u : 0u))) {
        flags |= 0x10;
    }
    if (((a ^ operand) & (a ^ result) & 0x80) != 0) {
        flags |= 0x04;
    }
    if ((result & 0x08) != 0) {
        flags |= 0x08;
    }
    if ((result & 0x20) != 0) {
        flags |= 0x20;
    }
    if (static_cast<uint16_t>(a) < subtrahend) {
        flags |= 0x01;
    }
    return flags;
}

uint8_t logical_flags(uint8_t result, bool half_carry) {
    uint8_t flags = parity_flag(result);
    if ((result & 0x80) != 0) {
        flags |= 0x80;
    }
    if (result == 0) {
        flags |= 0x40;
    }
    if (half_carry) {
        flags |= 0x10;
    }
    if ((result & 0x08) != 0) {
        flags |= 0x08;
    }
    if ((result & 0x20) != 0) {
        flags |= 0x20;
    }
    return flags;
}

uint8_t opcode_for(ImmOp op) {
    switch (op) {
        case ImmOp::Add:
            return 0xc6;
        case ImmOp::Adc:
            return 0xce;
        case ImmOp::Sub:
            return 0xd6;
        case ImmOp::Sbc:
            return 0xde;
        case ImmOp::And:
            return 0xe6;
        case ImmOp::Xor:
            return 0xee;
        case ImmOp::Or:
            return 0xf6;
        case ImmOp::Cp:
            return 0xfe;
    }
    return 0x00;
}

uint8_t expected_flags_for(ImmOp op, uint8_t a, uint8_t operand, bool carry_in) {
    const uint8_t result = [&]() -> uint8_t {
        switch (op) {
            case ImmOp::Add:
                return static_cast<uint8_t>(a + operand);
            case ImmOp::Adc:
                return static_cast<uint8_t>(a + operand + (carry_in ? 1 : 0));
            case ImmOp::Sub:
                return static_cast<uint8_t>(a - operand);
            case ImmOp::Sbc:
                return static_cast<uint8_t>(a - operand - (carry_in ? 1 : 0));
            case ImmOp::And:
                return static_cast<uint8_t>(a & operand);
            case ImmOp::Xor:
                return static_cast<uint8_t>(a ^ operand);
            case ImmOp::Or:
                return static_cast<uint8_t>(a | operand);
            case ImmOp::Cp:
                return static_cast<uint8_t>(a - operand);
        }
        return 0;
    }();

    switch (op) {
        case ImmOp::Add:
            return add_flags(a, operand, false, result);
        case ImmOp::Adc:
            return add_flags(a, operand, carry_in, result);
        case ImmOp::Sub:
            return sub_flags(a, operand, false, result);
        case ImmOp::Sbc:
            return sub_flags(a, operand, carry_in, result);
        case ImmOp::And:
            return logical_flags(result, true);
        case ImmOp::Xor:
        case ImmOp::Or:
            return logical_flags(result, false);
        case ImmOp::Cp:
            return static_cast<uint8_t>((sub_flags(a, operand, false, result) & ~0x28u) | (operand & 0x28u));
    }
    return 0;
}

uint8_t expected_accum_for(ImmOp op, uint8_t a, uint8_t operand, bool carry_in) {
    switch (op) {
        case ImmOp::Add:
            return static_cast<uint8_t>(a + operand);
        case ImmOp::Adc:
            return static_cast<uint8_t>(a + operand + (carry_in ? 1 : 0));
        case ImmOp::Sub:
            return static_cast<uint8_t>(a - operand);
        case ImmOp::Sbc:
            return static_cast<uint8_t>(a - operand - (carry_in ? 1 : 0));
        case ImmOp::And:
            return static_cast<uint8_t>(a & operand);
        case ImmOp::Xor:
            return static_cast<uint8_t>(a ^ operand);
        case ImmOp::Or:
            return static_cast<uint8_t>(a | operand);
        case ImmOp::Cp:
            return a;
    }
    return a;
}

struct OpCase {
    const char *name;
    ImmOp op;
    bool uses_carry_in;
};

void run_immediate_alu_matrix_case(const OpCase &op) {
    const uint8_t opcode = opcode_for(op.op);

    for (unsigned int a_raw = 0; a_raw <= 0xff; ++a_raw) {
        const uint8_t a = static_cast<uint8_t>(a_raw);
        for (unsigned int operand_raw = 0; operand_raw <= 0xff; ++operand_raw) {
            const uint8_t operand = static_cast<uint8_t>(operand_raw);
            for (bool carry_in : {false, true}) {
                if (!op.uses_carry_in && carry_in) {
                    continue;
                }

                CpuHarness h;
                INFO(op.name << " a=0x" << std::hex << static_cast<unsigned int>(a) << " operand=0x"
                             << static_cast<unsigned int>(operand) << " carry_in=" << carry_in);
                h.cpu.af.accum(a);
                h.cpu.af.flags(0x00);
                h.cpu.af.flag(RegisterAF::Flags::Carry, carry_in);
                h.load({opcode, operand});

                const StepResult step = h.step();

                REQUIRE(step.cycle_delta() == 7);
                REQUIRE(step.pc_after == 0x0002);
                REQUIRE(h.cpu.af.accum() == expected_accum_for(op.op, a, operand, carry_in));
                require_flags(h.cpu.af.flags(), expected_flags_for(op.op, a, operand, carry_in));
            }
        }
    }
}
}  // namespace

TEST_CASE("Immediate ADD opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"add a,n", ImmOp::Add, false});
}

TEST_CASE("Immediate ADC opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"adc a,n", ImmOp::Adc, true});
}

TEST_CASE("Immediate SUB opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"sub n", ImmOp::Sub, false});
}

TEST_CASE("Immediate SBC opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"sbc a,n", ImmOp::Sbc, true});
}

TEST_CASE("Immediate AND opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"and n", ImmOp::And, false});
}

TEST_CASE("Immediate XOR opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"xor n", ImmOp::Xor, false});
}

TEST_CASE("Immediate OR opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"or n", ImmOp::Or, false});
}

TEST_CASE("Immediate CP opcode matches full-byte flag formulas", "[alu][compliance][alu-immediate-matrix]") {
    run_immediate_alu_matrix_case({"cp n", ImmOp::Cp, false});
}
