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

struct TimedDisplayWrite {
    uint64_t frame_tstate = {0};
    uint16_t addr = {0};
    uint8_t old_value = {0};
    uint8_t value = {0};
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

    // Non-contentious mapped inspection. Emulated CPU writes must go through
    // write_data() so ROM mappings and contention stay authoritative.
    uint8_t operator[](uint16_t addr) const { return peek(addr); }
    uint8_t peek(uint16_t addr) const { return mapped_byte(addr); }

    // Test setup only: synthetic CPU programs often live in low memory where
    // the real machine maps ROM, so tests need an explicit bypass.
    void poke_mapped_for_test(uint16_t addr, uint8_t value) { mapped_byte(addr) = value; }

    uint8_t read_physical_ram(uint8_t bank, uint16_t offset) const {
        assert(bank < machine.ram_bank_count);
        return ram[bank_offset(bank, offset)];
    }
    void write_physical_ram(uint8_t bank, uint16_t offset, uint8_t value) {
        assert(bank < machine.ram_bank_count);
        ram[bank_offset(bank, offset)] = value;
    }
    void write_physical_ram_block(uint8_t bank, uint16_t offset, const uint8_t *data, std::size_t size) {
        assert(bank < machine.ram_bank_count);
        assert(offset <= bank_size);
        assert(size <= static_cast<std::size_t>(bank_size - offset));
        for (std::size_t pos = 0; pos < size; ++pos) {
            ram[bank_offset(bank, static_cast<uint16_t>(offset + pos))] = data[pos];
        }
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
    void clear_timed_display_writes() { timed_display_writes.clear(); }
    const std::vector<TimedDisplayWrite> &display_writes() const { return timed_display_writes; }

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
    void restore_memory_paging_register(uint8_t value) { set_memory_paging_register(value, true); }
    uint8_t selected_ay_register() const { return selected_ay_register_value; }
    uint8_t ay_register(uint8_t reg) const { return ay_register_values[reg & 0x0f]; }
    void reset_ay_state() {
        selected_ay_register_value = 0;
        ay_register_values.fill(0);
    }
    void restore_ay_state(uint8_t selected_register, const std::array<uint8_t, 16> &registers) {
        selected_ay_register_value = selected_register & 0x0f;
        ay_register_values = registers;
    }
    uint8_t kempston_joystick_state() const { return kempston_joystick_state_value; }
    void set_kempston_joystick_state(uint8_t value) { kempston_joystick_state_value = value & 0x1f; }
    uint8_t ula_screen_bank() const {
        return shadow_screen_enabled_value ? machine.shadow_screen_bank : machine.default_screen_bank;
    }
    uint8_t read_ula_screen(uint16_t addr) const {
        assert(addr >= machine.screen_bitmap_base);
        assert(addr < static_cast<uint16_t>(machine.screen_bitmap_base + bank_size));
        return read_physical_ram(ula_screen_bank(), static_cast<uint16_t>(addr - machine.screen_bitmap_base));
    }

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

        if (machine.family == MachineFamily::Spectrum128K) {
            apply_memory_paging_state();
            return;
        }

        cpu_pages[0] = PageMapping{PhysicalMemoryKind::Rom, 0};
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

    bool memory_paging_port_selected(uint16_t addr) const {
        // Original 128K paging is partially decoded: A15 and A1 must both be low.
        return machine.has_memory_paging && ((addr & 0x8002) == 0);
    }

    bool ay_register_port_selected(uint16_t addr) const {
        // Original 128K AY register select/read: A15=1, A14=1, A1=0.
        return machine.ay_frequency_hz != 0 && ((addr & 0xc002) == 0xc000);
    }

    bool ay_data_port_selected(uint16_t addr) const {
        // Original 128K AY data write: A15=1, A14=0, A1=0.
        return machine.ay_frequency_hz != 0 && ((addr & 0xc002) == 0x8000);
    }

    bool kempston_joystick_port_selected(uint16_t addr) const {
        // Common Kempston joystick reads use port 0x1f. Return no input unless
        // an input layer explicitly sets the state; do not fall through to the
        // floating bus because menu code can mistake random bits for directions.
        return (addr & 0x00ff) == 0x001f;
    }

    void write_memory_paging_register(uint8_t value) { set_memory_paging_register(value, false); }

    void set_memory_paging_register(uint8_t value, bool force) {
        if (memory_paging_disabled_value && !force) {
            return;
        }

        memory_paging_register_value = value;
        selected_paged_ram_bank_value = static_cast<uint8_t>(value & machine.paging_ram_bank_mask);
        shadow_screen_enabled_value = (value & machine.paging_shadow_screen_bit) != 0;
        selected_rom_bank_value = (value & machine.paging_rom_select_bit) != 0 ? 1 : 0;
        memory_paging_disabled_value = (value & machine.paging_disable_bit) != 0;
        apply_memory_paging_state();
    }

    void apply_memory_paging_state() {
        if (!machine.has_memory_paging) {
            return;
        }

        assert(selected_rom_bank_value < machine.rom_bank_count);
        assert(selected_paged_ram_bank_value < machine.ram_bank_count);
        assert(machine.default_screen_bank < machine.ram_bank_count);

        cpu_pages[0] = PageMapping{PhysicalMemoryKind::Rom, selected_rom_bank_value};
        cpu_pages[1] = PageMapping{PhysicalMemoryKind::Ram, machine.default_screen_bank};
        cpu_pages[2] = PageMapping{PhysicalMemoryKind::Ram, 2};
        cpu_pages[3] = PageMapping{PhysicalMemoryKind::Ram, selected_paged_ram_bank_value};
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
    std::array<uint8_t, 16> ay_register_values{};
    uint8_t selected_ay_register_value = {0};
    uint8_t kempston_joystick_state_value = {0};
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
    std::vector<TimedDisplayWrite> timed_display_writes;
};
