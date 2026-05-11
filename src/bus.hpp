/**
 * @brief Class managing memory/data bus of the system.
 */

#pragma once

#include <cassert>
#include <cstdint>
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
        : machine(_machine), mem(_machine.memory_size), ram_start(machine.ram_base) {}
    explicit Bus(size_t size) : machine(spectrum_48k_model()), mem(size), ram_start(machine.ram_base) {}
    virtual ~Bus() {}

    void load_rom(std::string &rom_file);
    void load_snapshot(std::string &sna_file, Z80 &state);
    void load_z80(std::string &z80_file, Z80 &state);

    uint8_t &operator[](uint16_t addr) { return mem[addr]; }

    uint8_t read_port(uint16_t addr) const;
    void write_port(uint16_t addr, uint8_t v);

    uint8_t read_data(uint16_t addr) const {
        account_contention(addr);
        return mem[addr];
    }

    void write_data(uint16_t addr, uint8_t v) {
        account_contention(addr);
        if (addr >= ram_start) {
            mem[addr] = v;
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
        return StorageElement(&mem[addr], count, (addr < ram_start));
    }

    FetchedOpcode read_opcode_from_mem(uint16_t addr) const;
    void set_frame_tstate(uint64_t tstate) {
        current_frame_tstate = tstate;
        frame_tstate_valid = true;
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

    void clock() {
        // Not actively used at the moment but may be useful for debugging
    }

    // TODO - this needs to be dealt with better at some point
    uint8_t port_254 = {0};
    mutable uint16_t floating_counter = {0};
    const MachineModel &model() const { return machine; }

private:
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
    std::vector<uint8_t> mem;
    uint16_t ram_start = {0};
    mutable bool ear_input_active = {true};
    mutable uint64_t current_frame_tstate = {0};
    mutable bool frame_tstate_valid = {false};
    mutable uint32_t contention_wait_states = {0};
    mutable uint32_t contention_access_phase = {0};
    mutable bool contention_active = {false};
};
