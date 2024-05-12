
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
    print("{:>32}: 0x{:04x}".format(reg_name, value+modify), end="")

def print_8bit_reg(data, offset, reg_name):
    """ Print 8-bit register to stdout """
    print("{:>32}:   0x{:02x}".format(reg_name, data[offset]), end="")


parser = argparse.ArgumentParser(description='Dump the register state stored in a ZX spectrum Z80 file.')
parser.add_argument('--file', type=argparse.FileType('rb'), help='The SNA file to dump')

args = parser.parse_args()

data = args.file.read()

# Read Header 1
print("Header 1")

print_8bit_reg(data, 0, "A")
print(" - ", end="")
print_8bit_reg(data, 1, "F")
print()

pc = read_16bit_size(data[6:])
print_16bit_reg(data, 2, "BC")
print()
print_16bit_reg(data, 4, "HL")
print()
print_16bit_reg(data, 6, "PC")
print()
print_16bit_reg(data, 8, "SP")
print()

print_8bit_reg(data, 10, "I")
print(" - ", end="")
print_8bit_reg(data, 11, "R")
print()

print()
byte_12 = data[12]
print_8bit_reg(data, 12, "Byte 12")
print()

print("   R reg bit 7: {}".format(byte_12 & 0x1))
print("  Border color: {}".format((byte_12 >> 1) & 0x7))
print("     SamRom on: {}".format((byte_12 >> 4) & 0x1))
print("Compression on: {}".format((byte_12 >> 5) & 0x1))
compression = bool((byte_12 >> 5) & 0x1)
print()

print_16bit_reg(data, 13, "DE")
print()
print_16bit_reg(data, 15, "BC\'")
print()
print_16bit_reg(data, 17, "DE\'")
print()
print_16bit_reg(data, 19, "HL\'")
print()

print_8bit_reg(data, 21, "A\'")
print(" - ", end="")
print_8bit_reg(data, 22, "F\'")
print()

print_16bit_reg(data, 23, "IY")
print()
print_16bit_reg(data, 25, "IX")
print()

print_8bit_reg(data, 27, "Int f/f")
print(" (Interrupt flipflop, 0=DI, otherwise EI)")

print_8bit_reg(data, 28, "IFF2")
print()

print()
byte_29 = data[29]
print_8bit_reg(data, 29, "Byte 29")
print()

print("    Interrupt mode: {}".format(byte_29 & 0x3))
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
size_header_2 = read_16bit_size(data[30:], little_endian=True)
print("Header 2")
print()
print("Header 2 size: {}".format(size_header_2))
print_16bit_reg(data, 0x1E, "Byte 30")
print()

if size_header_2 == 23:
    version = 2
elif size_header_2 == 54 or size_header_2 == 5:
    version = 3
else:
    version = "UNKNOWN (size {})".format(size_header_2)

print("Version {} detected".format(version))
print()

print_16bit_reg(data, 32, "PC")
print()

hardware_mode = data[34]
if version == 2:
    if hardware_mode == 0:
        hardware = "48k"
    elif hardware_mode == 1:
        hardware = "48k + If.1"
    elif hardware_mode == 2:
        hardware = "SamRam"
    elif hardware_mode == 3:
        hardware = "128k"
    elif hardware_mode == 4:
        hardware = "128k + If.1"
    else:
        hardware = "UNKNOWN H/W mode ({})".format(hardware_mode)
elif version == 3:
    if hardware_mode == 0:
        hardware = "48k"
    elif hardware_mode == 1:
        hardware = "48k + If.1"
    elif hardware_mode == 2:
        hardware = "SamRam"
    elif hardware_mode == 3:
        hardware = "48k + M.G.T."
    elif hardware_mode == 4:
        hardware = "128k"
    elif hardware_mode == 5:
        hardware = "128k + If.1"
    elif hardware_mode == 6:
        hardware = "128k + M.G.T."
    else:
        hardware = "UNKNOWN H/W mode ({})".format(hardware_mode)

print("Hardware mode: {}".format(hardware))

print_8bit_reg(data, 35, "Byte 35")
print()

print_8bit_reg(data, 36, "Intf 1 paged")
print()

print_8bit_reg(data, 37, "Emulation flags")
print()

print_8bit_reg(data, 38, "Last OUT to port 0xfffd")
print()

print_16bit_reg(data, 55, "Low T state counter")
print()

print_8bit_reg(data, 57, "Hi T state counter")
print()

print_8bit_reg(data, 58, "QL flag byte")
print()

print_8bit_reg(data, 59, "MGT Rom paged")
print()

print_8bit_reg(data, 60, "Multiface Rom paged")
print()

print_8bit_reg(data, 61, "0-8191 is ROM (if set)")
print()

print_8bit_reg(data, 62, "8192-16383 is ROM (if set)")
print()

print_8bit_reg(data, 83, "MGT type")
print()

print_8bit_reg(data, 84, "Disciple inhibit button")
print()

print_8bit_reg(data, 85, "Disciple inhibit flag")
print()

if size_header_2 == 55:
    print_8bit_reg(data, 86, "OUT to port 0x1ffd")
    print()
    data_pos = 87
else:
    data_pos = 86

while data_pos < len(data):
    data_length = read_16bit_size(data[data_pos:])
    page = data[data_pos + 2]
    is_compressed = True
    if data_length == 0xffff:
        is_compressed = False
        data_length = 16384

    print("Data block:")
    print("      Position: {} (0x{:08x})".format(data_pos, data_pos))
    print("        Length: {}".format(data_length))
    print(" Is Compressed: {}".format(is_compressed))
    page_desc = "UNKNOWN (page {})".format(page)
    page_data_48k = { 0: "48K rom",
                      1: "Interface I, Disciple or Plus D rom",
                      2: "- (page 2)",
                      3: "- (page 3)",
                      4: "8000-bfff",
                      5: "c000-ffff",
                      6: "- (page 6)",
                      7: "- (page 7)",
                      8: "4000-7fff",
                      9: "- (page 9)",
                      10: "- (page 10)",
                      11: "Multiface rom" }
    page_data_128k = { 0: "rom (basic)",
                       1: "Interface I, Disciple or Plus D rom",
                       2: "rom (reset)",
                       3: "page 0",
                       4: "page 1",
                       5: "page 2",
                       6: "page 3",
                       7: "page 4",
                       8: "page 5",
                       9: "page 6",
                       10: "page 7",
                       11: "Multiface rom" }

    if hardware_mode == 0 and page in page_data_48k:
        page_desc = page_data_48k[page]
    elif hardware_mode > 0 and page in page_data_128k:
        page_desc = page_data_128k[page]

    print("       Page {:2}: {}".format(page, page_desc))
    print()

    data_pos += 3 # Jump over header
    data_pos += data_length
