#include <array>
#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"

TEST_CASE("48K horizontal display layout matches the ULA timing table", "[bus]") {
    const MachineModel model = spectrum_48k_model();

    REQUIRE(model.family == MachineFamily::Spectrum48K);
    REQUIRE(model.memory_size == 65536);
    REQUIRE(model.physical_ram_size == 48 * 1024);
    REQUIRE(model.physical_rom_size == 16 * 1024);
    REQUIRE(model.bank_size == 0x4000);
    REQUIRE(model.ram_bank_count == 3);
    REQUIRE(model.rom_bank_count == 1);
    REQUIRE_FALSE(model.has_memory_paging);
    REQUIRE(model.cpu_frequency_hz == 3500000);
    REQUIRE(model.ay_frequency_hz == 0);
    REQUIRE(model.contention_visible_tstates == 128);
    REQUIRE(model.horizontal_border_left_tstates == 24);
    REQUIRE(model.horizontal_border_right_tstates == 24);
    REQUIRE(model.horizontal_blank_tstates == 48);
    REQUIRE(model.contention_visible_tstates + model.horizontal_border_right_tstates + model.horizontal_blank_tstates +
                model.horizontal_border_left_tstates ==
            model.contention_line_tstates);

    REQUIRE(model.horizontal_visible_border_left_tstates == 16);
    REQUIRE(model.horizontal_visible_border_right_tstates == 16);
    REQUIRE(model.horizontal_visible_border_left_tstates <= model.horizontal_border_left_tstates);
    REQUIRE(model.horizontal_visible_border_right_tstates <= model.horizontal_border_right_tstates);
    REQUIRE(model.border_left == model.horizontal_visible_border_left_tstates * 2);
    REQUIRE(model.border_right == model.horizontal_visible_border_right_tstates * 2);
    REQUIRE(model.visible_width() == model.border_left + model.screen_width + model.border_right);
    REQUIRE(model.visible_width() == 320);
    REQUIRE(model.border_top == 24);
    REQUIRE(model.visible_height() == 240);
}

TEST_CASE("Original 128K model records bank layout and 7C ULA timings", "[bus]") {
    const MachineModel model = spectrum_128k_model();

    REQUIRE(model.family == MachineFamily::Spectrum128K);
    REQUIRE(model.memory_size == 65536);
    REQUIRE(model.physical_ram_size == 128 * 1024);
    REQUIRE(model.physical_rom_size == 32 * 1024);
    REQUIRE(model.bank_size == 0x4000);
    REQUIRE(model.ram_bank_count == 8);
    REQUIRE(model.rom_bank_count == 2);
    REQUIRE(model.has_memory_paging);
    REQUIRE(model.memory_paging_port == 0x7ffd);
    REQUIRE(model.paging_ram_bank_mask == 0x07);
    REQUIRE(model.paging_shadow_screen_bit == 0x08);
    REQUIRE(model.paging_rom_select_bit == 0x10);
    REQUIRE(model.paging_disable_bit == 0x20);
    REQUIRE(model.default_screen_bank == 5);
    REQUIRE(model.shadow_screen_bank == 7);

    REQUIRE(model.cpu_frequency_hz == 3546900);
    REQUIRE(model.ay_frequency_hz == 1773400);
    REQUIRE(model.interrupt_hold_tstates == 36);
    REQUIRE(model.contention_first_tstate == 14362);
    REQUIRE(model.frame_tstates == 70908);
    REQUIRE(model.contention_visible_tstates == 128);
    REQUIRE(model.horizontal_border_left_tstates == 24);
    REQUIRE(model.horizontal_border_right_tstates == 24);
    REQUIRE(model.horizontal_blank_tstates == 52);
    REQUIRE(model.contention_visible_tstates + model.horizontal_border_right_tstates + model.horizontal_blank_tstates +
                model.horizontal_border_left_tstates ==
            model.contention_line_tstates);

    REQUIRE(model.default_rom_filename_count == 3);
}

TEST_CASE("128K bus maps CPU pages onto separate physical banks before paging is implemented", "[bus]") {
    Bus bus(spectrum_128k_model());

    REQUIRE(bus.memory_paging_register() == 0x00);
    REQUIRE(bus.selected_rom_bank() == 0);
    REQUIRE(bus.selected_paged_ram_bank() == 0);
    REQUIRE_FALSE(bus.shadow_screen_enabled());
    REQUIRE_FALSE(bus.memory_paging_disabled());

    bus.poke_mapped_for_test(0x0000, 0x11);
    bus.write_data(0x0000, 0x22);
    REQUIRE(bus[0x0000] == 0x11);
    REQUIRE(bus.read_physical_rom(0, 0x0000) == 0x11);

    bus.poke_mapped_for_test(0x4000, 0x33);
    bus.poke_mapped_for_test(0x8000, 0x44);
    bus.poke_mapped_for_test(0xc000, 0x55);

    REQUIRE(bus.read_physical_ram(5, 0x0000) == 0x33);
    REQUIRE(bus.read_physical_ram(2, 0x0000) == 0x44);
    REQUIRE(bus.read_physical_ram(0, 0x0000) == 0x55);

    bus.write_physical_ram(1, 0x0000, 0x66);
    REQUIRE(bus[0xc000] == 0x55);
}

TEST_CASE("128K paging register writes update state and latch the paging lock", "[bus]") {
    Bus bus(spectrum_128k_model());

    bus.write_port(0x7ffd, 0x1b);

    REQUIRE(bus.memory_paging_register() == 0x1b);
    REQUIRE(bus.selected_paged_ram_bank() == 3);
    REQUIRE(bus.selected_rom_bank() == 1);
    REQUIRE(bus.shadow_screen_enabled());
    REQUIRE_FALSE(bus.memory_paging_disabled());

    bus.write_port(0x7fff, 0x04);
    REQUIRE(bus.memory_paging_register() == 0x1b);

    bus.write_port(0x7ffd, 0x20);
    REQUIRE(bus.memory_paging_register() == 0x20);
    REQUIRE(bus.selected_paged_ram_bank() == 0);
    REQUIRE(bus.selected_rom_bank() == 0);
    REQUIRE_FALSE(bus.shadow_screen_enabled());
    REQUIRE(bus.memory_paging_disabled());

    bus.write_port(0x7ffd, 0x1f);
    REQUIRE(bus.memory_paging_register() == 0x20);
    REQUIRE(bus.selected_paged_ram_bank() == 0);
    REQUIRE(bus.selected_rom_bank() == 0);
    REQUIRE_FALSE(bus.shadow_screen_enabled());
}

TEST_CASE("128K paging register remaps the selected ROM and top RAM page", "[bus]") {
    Bus bus(spectrum_128k_model());

    bus.write_physical_rom(0, 0x0000, 0x11);
    bus.write_physical_rom(1, 0x0000, 0x22);
    bus.write_physical_ram(0, 0x0000, 0x33);
    bus.write_physical_ram(3, 0x0000, 0x44);

    REQUIRE(bus[0x0000] == 0x11);
    REQUIRE(bus[0xc000] == 0x33);

    bus.write_port(0x7ffd, 0x13);

    REQUIRE(bus[0x0000] == 0x22);
    REQUIRE(bus[0xc000] == 0x44);

    bus.write_data(0xc000, 0x55);
    REQUIRE(bus.read_physical_ram(3, 0x0000) == 0x55);
    REQUIRE(bus.read_physical_ram(0, 0x0000) == 0x33);

    bus.write_data(0x0000, 0x66);
    REQUIRE(bus.read_physical_rom(1, 0x0000) == 0x22);
}

TEST_CASE("128K snapshot paging restore bypasses the runtime paging lock", "[bus]") {
    Bus bus(spectrum_128k_model());

    bus.write_port(0x7ffd, 0x20);
    REQUIRE(bus.memory_paging_disabled());

    bus.restore_memory_paging_register(0x13);

    REQUIRE(bus.memory_paging_register() == 0x13);
    REQUIRE(bus.selected_paged_ram_bank() == 3);
    REQUIRE(bus.selected_rom_bank() == 1);
    REQUIRE_FALSE(bus.shadow_screen_enabled());
    REQUIRE_FALSE(bus.memory_paging_disabled());
}

TEST_CASE("128K ULA screen reads follow the shadow-screen paging bit", "[bus]") {
    Bus bus(spectrum_128k_model());

    bus.write_physical_ram(5, 0x0000, 0x12);
    bus.write_physical_ram(7, 0x0000, 0x34);

    REQUIRE(bus.ula_screen_bank() == 5);
    REQUIRE(bus.read_ula_screen(0x4000) == 0x12);
    REQUIRE(bus[0x4000] == 0x12);

    bus.write_port(0x7ffd, 0x08);

    REQUIRE(bus.ula_screen_bank() == 7);
    REQUIRE(bus.read_ula_screen(0x4000) == 0x34);
    REQUIRE(bus[0x4000] == 0x12);
}

TEST_CASE("Physical RAM block writes target an explicit bank", "[bus]") {
    Bus bus(spectrum_128k_model());
    const std::array<uint8_t, 3> bytes = {0x12, 0x34, 0x56};

    bus.write_physical_ram_block(4, 0x0100, bytes.data(), bytes.size());

    REQUIRE(bus.read_physical_ram(4, 0x0100) == 0x12);
    REQUIRE(bus.read_physical_ram(4, 0x0101) == 0x34);
    REQUIRE(bus.read_physical_ram(4, 0x0102) == 0x56);
    REQUIRE(bus.read_physical_ram(0, 0x0100) == 0x00);
}

TEST_CASE("128K AY register ports preserve selected register state", "[bus]") {
    Bus bus(spectrum_128k_model());

    bus.write_port(0xfffd, 0x0e);
    bus.write_port(0xbffd, 0x5a);

    REQUIRE(bus.selected_ay_register() == 0x0e);
    REQUIRE(bus.ay_register(0x0e) == 0x5a);
    REQUIRE(bus.read_port(0xfffd) == 0x5a);

    bus.write_port(0xfffd, 0x1f);
    bus.write_port(0xbffd, 0xa5);

    REQUIRE(bus.selected_ay_register() == 0x0f);
    REQUIRE(bus.ay_register(0x0f) == 0xa5);
    REQUIRE(bus.ay_register(0x0e) == 0x5a);
    REQUIRE(bus.read_port(0xfffd) == 0xa5);
}

TEST_CASE("Kempston joystick port returns explicit input state instead of floating bus data", "[bus]") {
    Bus bus(spectrum_128k_model());
    bus.poke_mapped_for_test(0x4000, 0xff);
    bus.floating_counter = 0;

    REQUIRE(bus.read_port(0x001f) == 0x00);
    REQUIRE(bus.floating_counter == 0);

    bus.set_kempston_joystick_state(0xff);

    REQUIRE(bus.kempston_joystick_state() == 0x1f);
    REQUIRE(bus.read_port(0x001f) == 0x1f);
    REQUIRE(bus.floating_counter == 0);
}

TEST_CASE("Bus preserves ROM write protection and RAM visibility", "[bus]") {
    Bus bus(65536);

    bus.poke_mapped_for_test(0x3fff, 0x11);
    bus.poke_mapped_for_test(0x4000, 0x22);

    bus.write_data(0x3fff, 0xaa);
    bus.write_data(0x4000, 0xbb);

    REQUIRE(bus.read_data(0x3fff) == 0x11);
    REQUIRE(bus.read_data(0x4000) == 0xbb);
}

TEST_CASE("Word writes respect the ROM to RAM boundary one byte at a time", "[bus]") {
    Bus bus(65536);

    bus.poke_mapped_for_test(0x3fff, 0x12);
    bus.poke_mapped_for_test(0x4000, 0x34);

    bus.write_addr_to_mem(0x3fff, 0xabcd);

    REQUIRE(bus.read_data(0x3fff) == 0x12);
    REQUIRE(bus.read_data(0x4000) == 0xab);
}

TEST_CASE("Timed display write recording is disabled by default", "[bus]") {
    Bus bus(65536);
    const uint16_t attr = bus.model().screen_attr_base;

    bus.set_frame_tstate(123);
    bus.write_data(attr, 0x42);

    REQUIRE_FALSE(bus.display_write_recording_enabled());
    REQUIRE(bus.display_writes().empty());
}

TEST_CASE("Timed display write recording captures attribute RAM writes", "[bus]") {
    Bus bus(65536);
    const uint16_t attr = bus.model().screen_attr_base;

    bus.poke_mapped_for_test(attr, 0x11);
    bus.set_display_write_recording_enabled(true);
    bus.set_frame_tstate(123);
    bus.write_data(attr, 0x42);

    REQUIRE(bus.display_writes().size() == 1);
    REQUIRE(bus.display_writes()[0].frame_tstate == 123);
    REQUIRE(bus.display_writes()[0].addr == attr);
    REQUIRE(bus.display_writes()[0].old_value == 0x11);
    REQUIRE(bus.display_writes()[0].value == 0x42);
}

TEST_CASE("Timed display write recording captures memory operand write-back", "[bus]") {
    Bus bus(65536);
    const uint16_t attr = bus.model().screen_attr_base;

    bus.poke_mapped_for_test(attr, 0x11);
    bus.set_display_write_recording_enabled(true);
    bus.set_frame_tstate(123);

    StorageElement elem = bus.read_element_from_mem(attr, 1);
    elem = static_cast<uint8_t>(0x42);

    REQUIRE(bus[attr] == 0x42);
    REQUIRE(bus.display_writes().size() == 1);
    REQUIRE(bus.display_writes()[0].frame_tstate == 123);
    REQUIRE(bus.display_writes()[0].addr == attr);
    REQUIRE(bus.display_writes()[0].old_value == 0x11);
    REQUIRE(bus.display_writes()[0].value == 0x42);
}

TEST_CASE("Timed display write recording ignores non-attribute writes", "[bus]") {
    Bus bus(65536);

    bus.set_display_write_recording_enabled(true);
    bus.set_frame_tstate(123);
    bus.write_data(bus.model().screen_bitmap_base, 0x42);
    bus.write_data(static_cast<uint16_t>(bus.model().screen_attr_base + 0x0300), 0x24);

    REQUIRE(bus.display_writes().empty());
}

TEST_CASE("ULA attribute reads hide future timed writes", "[bus]") {
    Bus bus(65536);
    const uint16_t attr = bus.model().screen_attr_base;

    bus.poke_mapped_for_test(attr, 0x11);
    bus.set_display_write_recording_enabled(true);
    bus.set_frame_tstate(100);
    bus.write_data(attr, 0x22);

    REQUIRE(bus.read_ula_attribute_at(attr, 99) == 0x11);
    REQUIRE(bus.read_ula_attribute_at(attr, 100) == 0x22);
    REQUIRE(bus.read_ula_attribute_at(attr, 101) == 0x22);
}

TEST_CASE("ULA attribute reads use the latest timed write at the sample point", "[bus]") {
    Bus bus(65536);
    const uint16_t attr = bus.model().screen_attr_base;

    bus.poke_mapped_for_test(attr, 0x11);
    bus.set_display_write_recording_enabled(true);
    bus.set_frame_tstate(100);
    bus.write_data(attr, 0x22);
    bus.set_frame_tstate(120);
    bus.write_data(attr, 0x33);

    REQUIRE(bus.read_ula_attribute_at(attr, 90) == 0x11);
    REQUIRE(bus.read_ula_attribute_at(attr, 110) == 0x22);
    REQUIRE(bus.read_ula_attribute_at(attr, 120) == 0x33);
    REQUIRE(bus.read_ula_attribute_at(attr, 130) == 0x33);
}

TEST_CASE("Port reads distinguish even keyboard ports from the floating bus path", "[bus]") {
    Bus bus(65536);
    bus.poke_mapped_for_test(0x4000, 0x81);
    bus.poke_mapped_for_test(0x4001, 0x42);
    bus.floating_counter = 0;

    const uint8_t even = bus.read_port(0x00fe);
    REQUIRE(even == 0xff);
    REQUIRE(bus.floating_counter == 0);

    const uint8_t first_odd = bus.read_port(0x00ff);
    const uint8_t second_odd = bus.read_port(0x00ff);
    REQUIRE(first_odd == 0x81);
    REQUIRE(second_odd == 0x42);
    REQUIRE(bus.floating_counter == 2);
}

TEST_CASE("Floating bus reads advance through the 16K display region and wrap around", "[bus]") {
    Bus bus(65536);
    bus.poke_mapped_for_test(0x4000, 0x11);
    bus.poke_mapped_for_test(0x4001, 0x22);
    bus.poke_mapped_for_test(0x7fff, 0x33);

    bus.floating_counter = 0x3fff;
    REQUIRE(bus.read_port(0x00ff) == 0x33);
    REQUIRE(bus.floating_counter == 0x4000);

    REQUIRE(bus.read_port(0x00ff) == 0x11);
    REQUIRE(bus.floating_counter == 0x4001);

    REQUIRE(bus.read_port(0x00ff) == 0x22);
    REQUIRE(bus.floating_counter == 0x4002);
}

TEST_CASE("Floating bus returns 0xff outside the active display window when beam timing is known", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(0);
    REQUIRE(bus.read_port(0x00ff) == 0xff);
}

TEST_CASE("Floating bus follows bitmap and attribute fetch phases during the display window", "[bus]") {
    Bus bus(65536);
    const uint16_t bitmap = bus.model().screen_bitmap_base;
    const uint16_t attr = bus.model().screen_attr_base;

    bus.poke_mapped_for_test(bitmap, 0x12);
    bus.poke_mapped_for_test(attr, 0x34);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    REQUIRE(bus.read_port(0x00ff) == 0x12);

    bus.set_frame_tstate(static_cast<uint64_t>(bus.model().contention_first_tstate + 2));
    REQUIRE(bus.read_port(0x00ff) == 0x34);
}

TEST_CASE("Even keyboard-port reads preserve the upper three bits regardless of row selection", "[bus]") {
    Bus bus(65536);

    const uint8_t all_rows = bus.read_port(0x00fe);
    const uint8_t middle_rows = bus.read_port(0xbffe);
    const uint8_t high_rows = bus.read_port(0x7ffe);

    REQUIRE((all_rows & 0xe0) == 0xe0);
    REQUIRE((middle_rows & 0xe0) == 0xe0);
    REQUIRE((high_rows & 0xe0) == 0xe0);
}

TEST_CASE("Port 0xfe exposes the EAR input on bit 6 while keeping the fixed high bits set", "[bus]") {
    Bus bus(65536);

    bus.set_input_line(MachineInputLine::Ear, true);
    REQUIRE(bus.input_line_active(MachineInputLine::Ear));
    REQUIRE((bus.read_port(0x00fe) & 0xe0) == 0xe0);

    bus.set_input_line(MachineInputLine::Ear, false);
    REQUIRE_FALSE(bus.input_line_active(MachineInputLine::Ear));
    REQUIRE((bus.read_port(0x00fe) & 0xe0) == 0xa0);
}

TEST_CASE("ROM writes remain blocked even when addressing through read-modify-write helpers", "[bus]") {
    Bus bus(65536);
    bus.poke_mapped_for_test(0x3ffe, 0x12);
    bus.poke_mapped_for_test(0x3fff, 0x34);
    bus.poke_mapped_for_test(0x4000, 0x56);

    bus.write_addr_to_mem(0x3ffe, 0xabcd);

    REQUIRE(bus.read_data(0x3ffe) == 0x12);
    REQUIRE(bus.read_data(0x3fff) == 0x34);
    REQUIRE(bus.read_data(0x4000) == 0x56);
}

TEST_CASE("Port writes only latch the Spectrum ULA port when the low byte is 0xfe", "[bus]") {
    Bus bus(65536);

    bus.port_254 = 0x00;
    bus.write_port(0x12fe, 0x77);
    REQUIRE(bus.port_254 == 0x77);
    REQUIRE(bus.beam_ula_port() == 0x77);

    bus.write_port(0x12ff, 0x33);
    REQUIRE(bus.port_254 == 0x77);
    REQUIRE(bus.beam_ula_port() == 0x77);
}

TEST_CASE("ULA port writes latch on the beam at the I/O write phase", "[bus]") {
    Bus bus(65536);

    bus.port_254 = 0x01;
    bus.set_frame_tstate(100);
    REQUIRE(bus.beam_ula_port() == 0x01);

    bus.begin_instruction_timing();
    bus.advance_instruction_timing(7);
    bus.write_port(0x00fe, 0x02);

    REQUIRE(bus.port_254 == 0x02);
    REQUIRE(bus.beam_ula_port() == 0x01);

    bus.set_frame_tstate(107);
    REQUIRE(bus.beam_ula_port() == 0x01);

    bus.set_frame_tstate(108);
    REQUIRE(bus.beam_ula_port() == 0x02);
    REQUIRE(bus.end_instruction_timing() == 0);
}

TEST_CASE("Block output port writes can delay the beam latch past the generic I/O phase", "[bus]") {
    Bus bus(65536);

    bus.port_254 = 0x01;
    bus.set_frame_tstate(100);
    REQUIRE(bus.beam_ula_port() == 0x01);

    bus.begin_instruction_timing();
    bus.advance_instruction_timing(7);
    bus.delay_next_beam_port_latch(bus.model().block_io_port_write_latch_extra_tstates);
    bus.write_port(0x00fe, 0x02);

    REQUIRE(bus.port_254 == 0x02);
    REQUIRE(bus.beam_ula_port() == 0x01);

    const uint64_t latch_tstate =
        100 + 7 + 1 + static_cast<uint64_t>(bus.model().block_io_port_write_latch_extra_tstates);

    bus.set_frame_tstate(latch_tstate - 1);
    REQUIRE(bus.beam_ula_port() == 0x01);

    bus.set_frame_tstate(latch_tstate);
    REQUIRE(bus.beam_ula_port() == 0x02);
    REQUIRE(bus.end_instruction_timing() == 0);
}

TEST_CASE("Delayed ULA port writes are queued rather than overwritten", "[bus]") {
    Bus bus(65536);

    bus.port_254 = 0x00;
    bus.set_frame_tstate(100);
    REQUIRE(bus.beam_ula_port() == 0x00);

    bus.begin_instruction_timing();
    bus.advance_instruction_timing(7);
    bus.delay_next_beam_port_latch(16);
    bus.write_port(0x00fe, 0x01);
    REQUIRE(bus.end_instruction_timing() == 0);

    bus.set_frame_tstate(116);
    bus.begin_instruction_timing();
    bus.advance_instruction_timing(7);
    bus.delay_next_beam_port_latch(16);
    bus.write_port(0x00fe, 0x02);
    REQUIRE(bus.end_instruction_timing() == 0);

    bus.set_frame_tstate(123);
    REQUIRE(bus.beam_ula_port() == 0x00);

    bus.set_frame_tstate(124);
    REQUIRE(bus.beam_ula_port() == 0x01);

    bus.set_frame_tstate(139);
    REQUIRE(bus.beam_ula_port() == 0x01);

    bus.set_frame_tstate(140);
    REQUIRE(bus.beam_ula_port() == 0x02);
}

TEST_CASE("Spectrum active-display border mapping is independent of the top viewport crop", "[bus]") {
    const auto model = spectrum_48k_model();

    REQUIRE(model.vertical_blank_top_lines == 24);
    REQUIRE(model.active_display_border_line_offset == 9);
    REQUIRE(model.border_top == 24);
    REQUIRE(model.visible_height() == 240);
}

TEST_CASE("Contention adds wait states for accesses in contended RAM during the active display window", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_data(bus.model().contention_ram_base);

    REQUIRE(bus.end_instruction_timing() == 6);
}

TEST_CASE("Contention does not add wait states outside contended RAM or outside the display window", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(0);
    bus.begin_instruction_timing();
    (void)bus.read_data(bus.model().contention_ram_base);
    REQUIRE(bus.end_instruction_timing() == 0);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base - 1));
    REQUIRE(bus.end_instruction_timing() == 0);
}

TEST_CASE("Contention follows the 8-tstate ULA delay pattern across successive accesses", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    for (int i = 0; i < 8; ++i) {
        (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base + i));
    }

    REQUIRE(bus.end_instruction_timing() == 21);
}

TEST_CASE("Contention uses the frame phase of each successive memory access within one instruction", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(static_cast<uint64_t>(bus.model().contention_first_tstate + 6));
    bus.begin_instruction_timing();
    (void)bus.read_data(bus.model().contention_ram_base);
    (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base + 1));
    (void)bus.read_data(static_cast<uint16_t>(bus.model().contention_ram_base + 2));

    REQUIRE(bus.end_instruction_timing() == 6);
}

TEST_CASE("Opcode fetch and operand reads both contribute to contention timing", "[bus]") {
    Bus bus(65536);

    const uint16_t base = bus.model().contention_ram_base;
    bus.poke_mapped_for_test(base, 0xdd);
    bus.poke_mapped_for_test(static_cast<uint16_t>(base + 1), 0xcb);
    bus.poke_mapped_for_test(static_cast<uint16_t>(base + 2), 0x05);
    bus.poke_mapped_for_test(static_cast<uint16_t>(base + 3), 0x46);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    const FetchedOpcode fetched = bus.read_opcode_from_mem(base);

    REQUIRE(fetched.opcode == 0xddcb46);
    REQUIRE(fetched.fetch_len == 3);
    REQUIRE(bus.end_instruction_timing() == 15);
}

TEST_CASE("Odd ports outside the contended address range do not add I/O wait states", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_port(0x00ff);

    REQUIRE(bus.end_instruction_timing() == 0);
}

TEST_CASE("Even ULA ports add the expected I/O contention pattern", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_port(0x00fe);

    REQUIRE(bus.end_instruction_timing() == 5);
}

TEST_CASE("Ports with a contended high byte incur full four-cycle I/O contention", "[bus]") {
    Bus bus(65536);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    (void)bus.read_port(0x40ff);
    REQUIRE(bus.end_instruction_timing() == 18);

    bus.set_frame_tstate(bus.model().contention_first_tstate);
    bus.begin_instruction_timing();
    bus.write_port(0x40fe, 0x03);
    REQUIRE(bus.end_instruction_timing() == 11);
}
