/**
 * @brief Header defining a register.
 */

#pragma once

#include <bitset>

#include "bus.hpp"
#include "common.hpp"
#include "storage_element.hpp"

/**
 * @brief Class describing a 16-bit register.
 */
class Register16 {
public:
    void reset() {
        reg = 0x00;
        alt_reg = 0x00;
    }
    void hi(uint8_t v) { c_reg[WORD_HI_BYTE_IDX] = v; }
    uint8_t hi() const { return c_reg[WORD_HI_BYTE_IDX]; }
    void lo(uint8_t v) { c_reg[WORD_LO_BYTE_IDX] = v; }
    uint8_t lo() const { return c_reg[WORD_LO_BYTE_IDX]; }
    void set(uint16_t v) { reg = v; }
    uint16_t get() const { return reg; }
    void swap() { std::swap(reg, alt_reg); }

    StorageElement element() { return StorageElement(&c_reg[0], 2); }
    StorageElement element_hi() { return StorageElement(&c_reg[WORD_HI_BYTE_IDX], 1); }
    StorageElement element_lo() { return StorageElement(&c_reg[WORD_LO_BYTE_IDX], 1); }

    friend std::ostream& operator<<(std::ostream& stream, const Register16& e);

private:
    union {
        uint16_t reg = 0;
        uint8_t c_reg[2];
    };
    uint16_t alt_reg = 0;
};

std::ostream& operator<<(std::ostream& stream, const Register16& e);

/**
 * @brief Specialization of Register16 for A and F registers.
 */
class RegisterAF : public Register16 {
public:
    enum class Flags {
        Carry = 0,
        AddSubtract = 1,
        ParityOverflow = 2,
        F3 = 3,
        HalfCarry = 4,
        F5 = 5,
        Zero = 6,
        Sign = 7
    };

    void accum(uint8_t v) { hi(v); }
    uint8_t accum() const { return hi(); }
    void flags(uint8_t v) { lo(v); }
    uint8_t flags() const { return lo(); }

    void flag(Flags f, bool v) {
        uint8_t bit = 0x1 << static_cast<unsigned int>(f);
        flags((flags() & ~bit) | (v ? bit : 0));
    }
    bool flag(Flags f) const {
        uint8_t bit = 0x1 << static_cast<unsigned int>(f);
        return (flags() & bit) != 0;
    }
    void inv_flag(Flags f) {
        if (flag(f))
            flag(f, false);
        else
            flag(f, true);
    }

    std::stringstream dump_flags();

    friend std::ostream& operator<<(std::ostream& stream, const RegisterAF& e);

private:
    bool sig_bit(unsigned int v, size_t count, bool ishalf = false) {
        unsigned int div = (ishalf ? 2 : 1);
        unsigned int mask = 0x1 << ((8 * count / div) - 1);
        return (v & mask) != 0;
    }
};

std::ostream& operator<<(std::ostream& stream, const RegisterAF& e);
