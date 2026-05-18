#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

class Z80;

struct TapeBlock {
    std::vector<uint8_t> bytes;

    uint8_t flag() const { return bytes.empty() ? 0xff : bytes.front(); }
    std::size_t data_size() const { return bytes.size() >= 2 ? bytes.size() - 2 : 0; }
    bool checksum_ok() const;
};

class TapeDeck {
public:
    bool load_tap(const std::string &filename);
    bool load_tap_stream(std::istream &stream);

    bool has_tape() const { return !blocks.empty(); }
    std::size_t block_count() const { return blocks.size(); }
    std::size_t current_block_index() const { return current_block; }

    bool try_fast_load(Z80 &state);

private:
    static constexpr uint16_t ld_bytes_entry = 0x0556;

    bool fast_load_block(Z80 &state, const TapeBlock &block);
    void return_from_rom_loader(Z80 &state, bool success);

    std::vector<TapeBlock> blocks;
    std::size_t current_block = 0;
};
