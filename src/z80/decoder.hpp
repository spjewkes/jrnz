/**
 * @brief Header defining the decoder implementation.
 */

#pragma once

#include <cstdint>
#include <string>

#include "instructions.hpp"

const Instruction& decode_opcode(uint32_t opcode);
bool has_rom_label(uint32_t address);
const std::string& decode_rom_label(uint32_t address);
