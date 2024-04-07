#!/usr/bin/env python3

import argparse

def read_16bit_size(data, little_endian=True):
    """ Read next two bytes as a 16-bit value """
    if little_endian:
        return data[0] + data[1] * 256
    else:
        return data[1] + data[0] * 256

def print_16bit_reg(data, offset, reg_name, modify=0):
    """ Print 16-bit register to stdout """
    value = read_16bit_size(data[offset:])
    print("{:>8}: 0x{:04x}".format(reg_name, value+modify), end="")

def print_8bit_reg(data, offset, reg_name):
    """ Print 8-bit register to stdout """
    print("{:>8}:   0x{:02x}".format(reg_name, data[offset]), end="")


parser = argparse.ArgumentParser(description='Dump the register state stored in a ZX spectrum Z80 file.')
parser.add_argument('--file', type=argparse.FileType('rb'), help='The SNA file to dump')

args = parser.parse_args()

data = args.file.read()

# Read Header 1
print("Header 1")

print_8bit_reg(data, 0x0, "A")
print(" - ", end="")
print_8bit_reg(data, 0x1, "F")
print()

pc = read_16bit_size(data[6:])
print_16bit_reg(data, 0x02, "BC")
print()
print_16bit_reg(data, 0x04, "HL")
print()
print_16bit_reg(data, 0x06, "PC")
print()
print_16bit_reg(data, 0x08, "SP")
print()

print_8bit_reg(data, 0x0A, "I")
print(" - ", end="")
print_8bit_reg(data, 0x0B, "R")
print()

print()
byte_12 = data[12]
print_8bit_reg(data, 0x0C, "Byte 12")
print()

print("   R reg bit 7: {}".format(byte_12 & 0x1))
print("  Border color: {}".format((byte_12 >> 1) & 0x7))
print("     SamRom on: {}".format((byte_12 >> 4) & 0x1))
print("Compression on: {}".format((byte_12 >> 5) & 0x1))
compression = bool((byte_12 >> 5) & 0x1)
print()

print_16bit_reg(data, 0x0D, "DE")
print()
print_16bit_reg(data, 0x0F, "BC\'")
print()
print_16bit_reg(data, 0x11, "DE\'")
print()
print_16bit_reg(data, 0x13, "HL\'")
print()

print_8bit_reg(data, 0x15, "A\'")
print(" - ", end="")
print_8bit_reg(data, 0x16, "F\'")
print()

print_16bit_reg(data, 0x17, "IY")
print()
print_16bit_reg(data, 0x19, "IX")
print()

print_8bit_reg(data, 0x1B, "Int f/f")
print(" (Interrupt flipflop, 0=DI, otherwise EI)")

print_8bit_reg(data, 0x1C, "IFF2")
print()

print()
byte_29 = data[29]
print_8bit_reg(data, 0x1D, "Byte 29")
print()

print("    Interrupt mode: {}".format(byte_29 & 0x2))
print("   Issue 2 enabled: {}".format((byte_29 >> 2) & 0x1))
print("Double int enabled: {}".format((byte_29 >> 3) & 0x1))
print("        Video sync: {}".format((byte_29 >> 4) & 0x3))
print("      Joystick sel: {}".format((byte_29 >> 6) & 0x3))
print()

version = 0
if byte_12 == 0xff or pc:
    version = 1
    print("Version 1 detected")
    quit()

# Read Header 2
size_header_2 = read_16bit_size(data[30:], little_endian=False)
print("Header 2 - size: {}".format(size_header_2))
print_16bit_reg(data, 0x1E, "Byte 30")
print()

# TODO need to understand this further. My test file is not reported version 1 but it looks like there is data after the first header and not the 2nd header
