#include "tape.hpp"

#include <fstream>
#include <iostream>

#include "bus.hpp"
#include "z80.hpp"

bool TapeBlock::checksum_ok() const {
    uint8_t checksum = 0;
    for (uint8_t byte : bytes) {
        checksum ^= byte;
    }
    return checksum == 0;
}

bool TapeDeck::load_tap(const std::string &filename) {
    std::ifstream stream(filename, std::ios::binary);
    if (!stream.is_open()) {
        std::cerr << "No TAP file found called '" << filename << "'\n";
        return false;
    }
    return load_tap_stream(stream);
}

bool TapeDeck::load_tap_stream(std::istream &stream) {
    blocks.clear();
    current_block = 0;

    while (stream.peek() != EOF) {
        uint8_t len_lo = 0;
        uint8_t len_hi = 0;
        stream.read(reinterpret_cast<char *>(&len_lo), 1);
        stream.read(reinterpret_cast<char *>(&len_hi), 1);
        if (!stream) {
            std::cerr << "TAP file ended in a partial block length\n";
            blocks.clear();
            return false;
        }

        const uint16_t length = static_cast<uint16_t>(len_lo | (len_hi << 8));
        TapeBlock block;
        block.bytes.resize(length);
        stream.read(reinterpret_cast<char *>(block.bytes.data()), static_cast<std::streamsize>(block.bytes.size()));
        if (!stream) {
            std::cerr << "TAP file ended in a partial block body\n";
            blocks.clear();
            return false;
        }

        blocks.push_back(std::move(block));
    }

    std::cout << "Loaded TAP with " << blocks.size() << " block(s)\n";
    return true;
}

bool TapeDeck::try_fast_load(Z80 &state) {
    if (!has_tape() || state.cycles_left != 0 || state.pc.get() != ld_bytes_entry) {
        return false;
    }

    bool success = false;
    if (current_block < blocks.size()) {
        success = fast_load_block(state, blocks[current_block]);
        ++current_block;
    } else {
        std::cerr << "TAP fast-load failed: tape is exhausted\n";
    }

    return_from_rom_loader(state, success);
    return true;
}

bool TapeDeck::fast_load_block(Z80 &state, const TapeBlock &block) {
    const uint8_t expected_flag = state.af.accum();
    const bool load = state.af.flag(RegisterAF::Flags::Carry);
    const uint16_t dest = state.ix.get();
    const uint16_t length = state.de.get();

    if (block.bytes.size() != static_cast<std::size_t>(length) + 2) {
        std::cerr << "TAP fast-load failed: expected " << length << " data byte(s), got " << block.data_size() << "\n";
        return false;
    }
    if (block.flag() != expected_flag) {
        std::cerr << "TAP fast-load failed: expected flag 0x" << std::hex << static_cast<unsigned int>(expected_flag)
                  << ", got 0x" << static_cast<unsigned int>(block.flag()) << std::dec << "\n";
        return false;
    }
    if (!block.checksum_ok()) {
        std::cerr << "TAP fast-load failed: checksum mismatch\n";
        return false;
    }

    bool verified = true;
    for (uint16_t offset = 0; offset < length; ++offset) {
        const uint8_t value = block.bytes[static_cast<std::size_t>(offset) + 1];
        const uint16_t addr = static_cast<uint16_t>(dest + offset);
        if (load) {
            state.bus.write_data(addr, value);
        } else if (state.bus.peek(addr) != value) {
            verified = false;
        }
    }

    if (!verified) {
        std::cerr << "TAP fast-load failed: verify mismatch\n";
        return false;
    }

    state.ix.set(static_cast<uint16_t>(dest + length));
    state.de.set(0);
    state.af.accum(0);
    return true;
}

void TapeDeck::return_from_rom_loader(Z80 &state, bool success) {
    const uint16_t return_addr = state.bus.read_addr_from_mem(state.sp.get());
    state.sp.set(static_cast<uint16_t>(state.sp.get() + 2));
    state.pc.set(return_addr);
    state.af.flag(RegisterAF::Flags::Carry, success);
}
