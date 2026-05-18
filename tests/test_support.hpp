#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>

#include "bus.hpp"
#include "z80.hpp"

struct StepResult {
    uint16_t pc_before;
    uint16_t pc_after;
    uint64_t cycles_before;
    uint64_t cycles_after;

    uint64_t cycle_delta() const { return cycles_after - cycles_before; }
};

struct CpuHarness {
    Bus mem;
    Z80 cpu;

    CpuHarness() : mem(65536), cpu(mem, true) {
        cpu.pc.set(0x0000);
        cpu.sp.set(0xfffe);
    }

    void load(std::initializer_list<uint8_t> bytes, uint16_t start = 0x0000) {
        size_t offset = 0;
        for (uint8_t byte : bytes) {
            mem.poke_mapped_for_test(static_cast<uint16_t>(start + offset), byte);
            ++offset;
        }
        cpu.pc.set(start);
    }

    StepResult step() {
        StepResult result{cpu.pc.get(), 0, cpu.total_cycles, 0};
        const bool ok = cpu.clock();
        REQUIRE(ok);
        result.pc_after = cpu.pc.get();
        result.cycles_after = cpu.total_cycles;
        return result;
    }
};

inline void require_f3_f5(const CpuHarness &h, bool f3, bool f5) {
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::F3) == f3);
    REQUIRE(h.cpu.af.flag(RegisterAF::Flags::F5) == f5);
}

inline void require_flags(uint8_t actual, uint8_t expected) {
    INFO("actual flags=0x" << std::hex << static_cast<unsigned int>(actual) << ", expected=0x"
                           << static_cast<unsigned int>(expected));
    REQUIRE(actual == expected);
}
