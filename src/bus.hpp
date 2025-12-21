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
#include "storage_element.hpp"

/**
 * Forward prototypes.
 */
class Z80;

/**
 * @brief Defines the memory/data bus of the device.
 */
class Bus {
public:
    Bus(size_t size) : mem(size) {}
    virtual ~Bus() {}

    void load_rom(std::string &rom_file);
    void load_snapshot(std::string &sna_file, Z80 &state);
    void load_z80(std::string &z80_file, Z80 &state);

    uint8_t &operator[](uint16_t addr) { return mem[addr]; }

    uint8_t read_port(uint16_t addr) const;
    void write_port(uint16_t addr, uint8_t v);

    uint8_t read_data(uint16_t addr) const { return mem[addr]; }

    void write_data(uint16_t addr, uint8_t v) {
        if (addr >= ram_start) {
            mem[addr] = v;
        }
    }

    uint16_t read_addr_from_mem(uint16_t addr) const {
        uint16_t ret_addr = mem[addr];
        ret_addr |= mem[addr + 1] << 8;
        return ret_addr;
    }
    void write_addr_to_mem(uint16_t addr, uint16_t addr_to_write) {
        write_data(addr, addr_to_write & 0xff);
        write_data(addr + 1, (addr_to_write >> 8) & 0xff);
    }

    StorageElement read_element_from_mem(uint16_t addr, size_t count) {
        return StorageElement(&mem[addr], count, (addr < ram_start));
    }

    uint32_t read_opcode_from_mem(uint16_t addr, uint16_t *operand_offset = nullptr);

    void clock() {
        // Not actively used at the moment but may be useful for debugging
    }

    // TODO - this needs to be dealt with better at some point
    uint8_t port_254 = {0};
    mutable uint16_t floating_counter = {0};

private:
    std::vector<uint8_t> mem;
    uint16_t ram_start = {0x4000};
};
