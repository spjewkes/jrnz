/**
 * @brief Class managing memory/data bus of the system.
 */

#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "common.hpp"
#include "keyboard.hpp"
#include "machine_config.hpp"
#include "storage_element.hpp"

/**
 * Forward prototypes.
 */
class Z80;

enum class MachineInputLine : uint8_t {
    Ear = 0,
};

struct FetchedOpcode {
    uint32_t opcode = {0};
    uint16_t operand_offset = {0};
    uint8_t fetch_len = {0};
    uint8_t ignored_prefixes = {0};
};

/**
 * @brief Defines the memory/data bus of the device.
 */
class Bus {
public:
    explicit Bus(const MachineModel &_machine)
        : machine(_machine), rom(machine.physical_rom_size), ram(machine.physical_ram_size) {
        configure_default_memory_map();
    }
    explicit Bus(size_t size) : Bus(spectrum_48k_model()) {
        assert(size == machine.memory_size);
        UNUSED(size);
    }
    virtual ~Bus() {}

    void load_rom(std::string &rom_file);
    void load_snapshot(std::string &sna_file, Z80 &state);
    void load_z80(std::string &z80_file, Z80 &state);

    uint8_t &operator[](uint16_t addr) { return mapped_byte(addr); }
    const uint8_t &operator[](uint16_t addr) const { return mapped_byte(addr); }

    uint8_t read_physical_ram(uint8_t bank, uint16_t offset) const {
        assert(bank < machine.ram_bank_count);
        return ram[bank_offset(bank, offset)];
    }
    void write_physical_ram(uint8_t bank, uint16_t offset, uint8_t value) {
        assert(bank < machine.ram_bank_count);
        ram[bank_offset(bank, offset)] = value;
    }
    uint8_t read_physical_rom(uint8_t bank, uint16_t offset) const {
        assert(bank < machine.rom_bank_count);
        return rom[bank_offset(bank, offset)];
    }
    void write_physical_rom(uint8_t bank, uint16_t offset, uint8_t value) {
        assert(bank < machine.rom_bank_count);
        rom[bank_offset(bank, offset)] = value;
    }

    uint8_t read_port(uint16_t addr) const;
    void write_port(uint16_t addr, uint8_t v);

    uint8_t read_data(uint16_t addr) const {
        account_contention(addr);
        return mapped_byte(addr);
    }

    void write_data(uint16_t addr, uint8_t v) {
        account_contention(addr);
        if (!is_read_only_addr(addr)) {
            mapped_byte(addr) = v;
        }
    }

    uint16_t read_addr_from_mem(uint16_t addr) const {
        uint16_t ret_addr = read_data(addr);
        ret_addr |= read_data(addr + 1) << 8;
        return ret_addr;
    }
    void write_addr_to_mem(uint16_t addr, uint16_t addr_to_write) {
        write_data(addr, addr_to_write & 0xff);
        write_data(addr + 1, (addr_to_write >> 8) & 0xff);
    }

    StorageElement read_element_from_mem(uint16_t addr, size_t count) {
        if (count > 1) {
            assert(page_index(addr) == page_index(static_cast<uint16_t>(addr + count - 1)));
        }
        return StorageElement(&mapped_byte(addr), count, is_read_only_addr(addr));
    }

    FetchedOpcode read_opcode_from_mem(uint16_t addr) const;
    void set_frame_tstate(uint64_t tstate) {
        current_frame_tstate = tstate;
        frame_tstate_valid = true;
        bool applied_delayed_write = false;
        for (auto it = pending_beam_port_254_writes.begin(); it != pending_beam_port_254_writes.end();) {
            if (current_frame_tstate == it->frame_tstate) {
                beam_port_254 = it->value;
                applied_delayed_write = true;
                it = pending_beam_port_254_writes.erase(it);
            } else {
                ++it;
            }
        }
        if (!applied_delayed_write && pending_beam_port_254_writes.empty() && !contention_active) {
            beam_port_254 = port_254;
        }
    }
    void set_input_line(MachineInputLine line, bool active) {
        switch (line) {
            case MachineInputLine::Ear:
                ear_input_active = active;
                break;
        }
    }
    bool input_line_active(MachineInputLine line) const {
        switch (line) {
            case MachineInputLine::Ear:
                return ear_input_active;
        }
        return false;
    }
    void set_ear_input(bool active) { set_input_line(MachineInputLine::Ear, active); }
    void begin_instruction_timing() {
        contention_active = true;
        contention_wait_states = 0;
        contention_access_phase = 0;
    }
    uint32_t end_instruction_timing() {
        contention_active = false;
        return contention_wait_states;
    }
    void advance_instruction_timing(uint32_t tstates) const {
        if (!contention_active) {
            return;
        }
        contention_access_phase += tstates;
    }
    void delay_next_beam_port_latch(uint32_t tstates) { next_beam_port_latch_extra_tstates += tstates; }

    void clock() {
        // Not actively used at the moment but may be useful for debugging
    }

    // TODO - this needs to be dealt with better at some point
    uint8_t port_254 = {0};
    // Beam-visible latch for border/audio timing; this can intentionally lag
    // behind port_254 while an in-flight I/O write reaches the ULA.
    uint8_t beam_ula_port() const { return beam_port_254; }
    mutable uint16_t floating_counter = {0};
    const MachineModel &model() const { return machine; }
    uint8_t memory_paging_register() const { return memory_paging_register_value; }
    uint8_t selected_rom_bank() const { return selected_rom_bank_value; }
    uint8_t selected_paged_ram_bank() const { return selected_paged_ram_bank_value; }
    bool shadow_screen_enabled() const { return shadow_screen_enabled_value; }
    bool memory_paging_disabled() const { return memory_paging_disabled_value; }

private:
    static constexpr uint16_t bank_size = 0x4000;
    static constexpr uint8_t cpu_page_count = 4;

    enum class PhysicalMemoryKind : uint8_t {
        Rom,
        Ram,
    };

    struct PageMapping {
        PhysicalMemoryKind kind;
        uint8_t bank;
    };

    static constexpr uint8_t page_index(uint16_t addr) { return static_cast<uint8_t>(addr / bank_size); }
    static constexpr uint16_t page_offset(uint16_t addr) { return static_cast<uint16_t>(addr & (bank_size - 1)); }

    std::size_t bank_offset(uint8_t bank, uint16_t offset) const {
        assert(offset < bank_size);
        return (static_cast<std::size_t>(bank) * bank_size) + offset;
    }

    bool is_read_only_addr(uint16_t addr) const { return cpu_pages[page_index(addr)].kind == PhysicalMemoryKind::Rom; }

    uint8_t &mapped_byte(uint16_t addr) {
        const PageMapping page = cpu_pages[page_index(addr)];
        const std::size_t offset = bank_offset(page.bank, page_offset(addr));
        return page.kind == PhysicalMemoryKind::Rom ? rom[offset] : ram[offset];
    }

    const uint8_t &mapped_byte(uint16_t addr) const {
        const PageMapping page = cpu_pages[page_index(addr)];
        const std::size_t offset = bank_offset(page.bank, page_offset(addr));
        return page.kind == PhysicalMemoryKind::Rom ? rom[offset] : ram[offset];
    }

    void configure_default_memory_map() {
        assert(machine.bank_size == bank_size);
        assert(rom.size() >= static_cast<std::size_t>(machine.rom_bank_count) * bank_size);
        assert(ram.size() >= static_cast<std::size_t>(machine.ram_bank_count) * bank_size);

        cpu_pages[0] = PageMapping{PhysicalMemoryKind::Rom, 0};

        if (machine.family == MachineFamily::Spectrum128K) {
            cpu_pages[1] = PageMapping{PhysicalMemoryKind::Ram, machine.default_screen_bank};
            cpu_pages[2] = PageMapping{PhysicalMemoryKind::Ram, 2};
            cpu_pages[3] = PageMapping{PhysicalMemoryKind::Ram, 0};
            return;
        }

        cpu_pages[1] = PageMapping{PhysicalMemoryKind::Ram, 0};
        cpu_pages[2] = PageMapping{PhysicalMemoryKind::Ram, 1};
        cpu_pages[3] = PageMapping{PhysicalMemoryKind::Ram, 2};
    }

    uint16_t floating_bus_addr_for_tstate(uint64_t tstate) const {
        const uint64_t frame_pos = tstate % machine.frame_tstates;
        const uint64_t visible_span = static_cast<uint64_t>(machine.contention_lines) * machine.contention_line_tstates;

        if (frame_pos < machine.contention_first_tstate ||
            frame_pos >= static_cast<uint64_t>(machine.contention_first_tstate) + visible_span) {
            return 0xffff;
        }

        const uint64_t visible_pos = frame_pos - machine.contention_first_tstate;
        const uint64_t line = visible_pos / machine.contention_line_tstates;
        const uint64_t line_pos = visible_pos % machine.contention_line_tstates;

        if (line_pos >= machine.contention_visible_tstates) {
            return 0xffff;
        }

        const uint16_t y = static_cast<uint16_t>(line);
        uint16_t screen_y = static_cast<uint16_t>(0xc0 & y);
        screen_y |= static_cast<uint16_t>((y & 0x7) << 3);
        screen_y |= static_cast<uint16_t>((y >> 3) & 0x7);

        const uint16_t column = static_cast<uint16_t>(line_pos / 4);
        const uint16_t bitmap_addr = static_cast<uint16_t>(machine.screen_bitmap_base + (screen_y * 32) + column);
        const uint16_t attr_addr = static_cast<uint16_t>(machine.screen_attr_base + ((screen_y >> 3) * 32) + column);

        return ((line_pos & 0x2) == 0) ? bitmap_addr : attr_addr;
    }

    void account_port_contention(uint16_t addr) const {
        if (!contention_active) {
            return;
        }

        const bool ula_port_selected = (addr & 0x1) == 0;
        const uint8_t high_byte = static_cast<uint8_t>(addr >> 8);
        const bool high_byte_contended = high_byte >= 0x40 && high_byte <= 0x7f;

        if (!ula_port_selected && !high_byte_contended) {
            contention_access_phase += 4;
            return;
        }

        auto contended_segment = [&](uint32_t duration) {
            const uint64_t sample_tstate = current_frame_tstate + contention_access_phase;
            const uint8_t delay = contention_delay(sample_tstate);
            contention_wait_states += delay;
            contention_access_phase += duration;
        };

        auto normal_segment = [&](uint32_t duration) { contention_access_phase += duration; };

        if (!high_byte_contended && ula_port_selected) {
            // N:1, C:3
            normal_segment(1);
            contended_segment(3);
            return;
        }

        if (high_byte_contended && ula_port_selected) {
            // C:1, C:3
            contended_segment(1);
            contended_segment(3);
            return;
        }

        // C:1, C:1, C:1, C:1
        contended_segment(1);
        contended_segment(1);
        contended_segment(1);
        contended_segment(1);
    }

    uint32_t port_write_latch_delay(uint16_t addr) const {
        const uint8_t high_byte = static_cast<uint8_t>(addr >> 8);
        const bool high_byte_contended = high_byte >= 0x40 && high_byte <= 0x7f;

        uint32_t delay = 1;
        if (high_byte_contended) {
            delay += contention_delay(current_frame_tstate + contention_access_phase);
        }
        return delay;
    }

    struct PendingBeamPortWrite {
        uint64_t frame_tstate;
        uint8_t value;
    };

    void schedule_beam_port_write(uint8_t value, uint32_t phase_delay) {
        pending_beam_port_254_writes.push_back(
            PendingBeamPortWrite{(current_frame_tstate + phase_delay) % machine.frame_tstates, value});
    }

    void account_contention(uint16_t addr) const {
        if (!contention_active) {
            return;
        }
        if (addr < machine.contention_ram_base || addr >= machine.contention_ram_end) {
            return;
        }

        contention_wait_states += contention_delay(current_frame_tstate + contention_access_phase);
        contention_access_phase += 1;
    }

    uint8_t contention_delay(uint64_t tstate) const {
        const uint64_t frame_pos = tstate % machine.frame_tstates;
        const uint64_t contention_span =
            static_cast<uint64_t>(machine.contention_lines) * machine.contention_line_tstates;

        if (frame_pos < machine.contention_first_tstate ||
            frame_pos >= static_cast<uint64_t>(machine.contention_first_tstate) + contention_span) {
            return 0;
        }

        const uint64_t line_pos = (frame_pos - machine.contention_first_tstate) % machine.contention_line_tstates;
        if (line_pos >= machine.contention_visible_tstates) {
            return 0;
        }

        return machine.contention_pattern[line_pos & 0x7];
    }

    MachineModel machine;
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;
    std::array<PageMapping, cpu_page_count> cpu_pages{};
    uint8_t memory_paging_register_value = {0};
    uint8_t selected_rom_bank_value = {0};
    uint8_t selected_paged_ram_bank_value = {0};
    bool shadow_screen_enabled_value = {false};
    bool memory_paging_disabled_value = {false};
    mutable bool ear_input_active = {true};
    mutable uint64_t current_frame_tstate = {0};
    mutable bool frame_tstate_valid = {false};
    mutable uint32_t contention_wait_states = {0};
    mutable uint32_t contention_access_phase = {0};
    mutable bool contention_active = {false};
    uint8_t beam_port_254 = {0};
    std::deque<PendingBeamPortWrite> pending_beam_port_254_writes;
    uint32_t next_beam_port_latch_extra_tstates = {0};
};
