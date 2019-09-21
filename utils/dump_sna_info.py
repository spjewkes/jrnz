#!/usr/bin/env python3

import argparse

def read_16bit_size(data, little_endian=True):
    """ Read next two bytes as a 16-bit value """
    if little_endian:
        return data[0] + data[1] * 256
    else:
        return data[1] + data[0] * 256

def print_16bit_reg(data, offset, reg_name):
    """ Print 16-bit register to stdout """
    value = read_16bit_size(data[offset:])
    print("{:>4}: 0x{:04x}".format(reg_name, value), end="")

def print_8bit_reg(data, offset, reg_name):
    """ Print 8-bit register to stdout """
    print("{:>4}:   0x{:02x}".format(reg_name, data[offset]), end="")
          

parser = argparse.ArgumentParser(description='Dump the register state stored in a ZX spectrum SNA file.')
parser.add_argument('--file', type=argparse.FileType('rb'), help='The SNA file to dump')

args = parser.parse_args()

data = args.file.read()


print_8bit_reg(data, 0x00, "I")
print("  -  ", end="")
print_8bit_reg(data, 0x14, "R")
print()

print_16bit_reg(data, 0x09, "HL")
print("  -  ", end="")
print_16bit_reg(data, 0x01, "HL\'")
print()

print_16bit_reg(data, 0x0B, "DE")
print("  -  ", end="")
print_16bit_reg(data, 0x03, "DE\'")
print()

print_16bit_reg(data, 0x0D, "BC")
print("  -  ", end="")
print_16bit_reg(data, 0x05, "BC\'")
print()

print_16bit_reg(data, 0x15, "AF")
print("  -  ", end="")
print_16bit_reg(data, 0x07, "AF\'")
print()

print_16bit_reg(data, 0x0f, "IY")
print("  -  ", end="")
print_16bit_reg(data, 0x11, "IX")
print()

print_16bit_reg(data, 0x17, "SP")
print("  -  ", end="")
# Fetch the PC from the SP. Skip over the 27 bytes of the header and
# offset the SP address by the ROM as this isn't part of the snapshot
sp = read_16bit_size(data[0x17:])
print_16bit_reg(data[27:], sp-16384, "PC")
print()

print_8bit_reg(data, 0x13, "IFF2")
print()

print_8bit_reg(data, 0x19, "IM")
print()

print_8bit_reg(data, 0x1a, "Border Color")
print()

