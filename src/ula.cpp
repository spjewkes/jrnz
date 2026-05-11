/**
 * Brief Implementation of the ULA class.
 */

#include "ula.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>

#include <algorithm>
#include <cstdint>

#include "common.hpp"

extern SDL_Window *window;
extern SDL_Renderer *renderer;

#ifdef HAVE_DISPLAY
static bool get_bit(uint8_t byte, uint8_t pos) { return (byte & (1 << pos)) != 0; }

static void set_rendercolor(SDL_Renderer *renderer, uint8_t color, bool bright) {
    if (bright) {
        switch (color) {
            case 0:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                break;
            case 1:
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
                break;
            case 4:
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
                break;
            case 5:
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
                break;
            case 6:
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
                break;
            case 7:
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                break;
        }
    } else {
        switch (color) {
            case 0:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                break;
            case 1:
                SDL_SetRenderDrawColor(renderer, 0, 0, 215, SDL_ALPHA_OPAQUE);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer, 215, 0, 0, SDL_ALPHA_OPAQUE);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer, 215, 0, 215, SDL_ALPHA_OPAQUE);
                break;
            case 4:
                SDL_SetRenderDrawColor(renderer, 0, 215, 0, SDL_ALPHA_OPAQUE);
                break;
            case 5:
                SDL_SetRenderDrawColor(renderer, 0, 215, 215, SDL_ALPHA_OPAQUE);
                break;
            case 6:
                SDL_SetRenderDrawColor(renderer, 215, 215, 0, SDL_ALPHA_OPAQUE);
                break;
            case 7:
                SDL_SetRenderDrawColor(renderer, 215, 215, 215, SDL_ALPHA_OPAQUE);
                break;
        }
    }
}
#endif

uint8_t ULA::remap_spectrum_y(uint8_t y) {
    uint8_t remapped = static_cast<uint8_t>(0xc0 & y);
    remapped |= static_cast<uint8_t>((y & 0x7) << 3);
    remapped |= static_cast<uint8_t>((y >> 3) & 0x7);
    return remapped;
}

void ULA::record_border_tstate(uint64_t frame_pos) {
    const uint64_t visible_span = static_cast<uint64_t>(machine.visible_height()) * machine.contention_line_tstates;
    if (frame_pos < visible_frame_start_tstate ||
        frame_pos >= static_cast<uint64_t>(visible_frame_start_tstate) + visible_span) {
        return;
    }

    const uint64_t visible_pos = frame_pos - visible_frame_start_tstate;
    const uint64_t line = visible_pos / machine.contention_line_tstates;
    const uint64_t line_pos = visible_pos % machine.contention_line_tstates;
    if (line_pos >= horizontal_visible_tstates()) {
        return;
    }

    border_timeline[(line * horizontal_visible_tstates()) + line_pos] = static_cast<uint8_t>(_bus.port_254 & 0x07);
}

void ULA::record_screen_tstate(uint64_t frame_pos) {
    const uint64_t visible_span = static_cast<uint64_t>(machine.visible_height()) * machine.contention_line_tstates;
    if (frame_pos < machine.contention_first_tstate ||
        frame_pos >= static_cast<uint64_t>(visible_frame_start_tstate) + visible_span) {
        return;
    }

    const uint64_t visible_pos = frame_pos - visible_frame_start_tstate;
    const uint64_t line = visible_pos / machine.contention_line_tstates;
    const uint64_t line_pos = visible_pos % machine.contention_line_tstates;
    if (line < static_cast<uint64_t>(machine.border_top) ||
        line >= static_cast<uint64_t>(machine.border_top + machine.screen_height)) {
        return;
    }
    if (line_pos !=
        static_cast<uint64_t>(machine.horizontal_border_left_tstates + machine.contention_visible_tstates - 1)) {
        return;
    }

    const std::size_t bytes_per_line = static_cast<std::size_t>(machine.screen_width / machine.attr_cell_size);
    const uint8_t display_y = static_cast<uint8_t>(line - machine.border_top);
    const uint8_t memory_y = remap_spectrum_y(display_y);
    const uint16_t bitmap_addr = static_cast<uint16_t>(machine.screen_bitmap_base + (memory_y * bytes_per_line));
    const uint16_t attr_addr = static_cast<uint16_t>(machine.screen_attr_base + ((display_y >> 3) * bytes_per_line));
    const std::size_t line_offset = static_cast<std::size_t>(display_y) * bytes_per_line;

    for (std::size_t x = 0; x < bytes_per_line; ++x) {
        screen_bitmap_snapshot[line_offset + x] = _bus[static_cast<uint16_t>(bitmap_addr + x)];
        screen_attr_snapshot[line_offset + x] = _bus[static_cast<uint16_t>(attr_addr + x)];
    }
}

void ULA::render_frame() const {
#ifdef HAVE_DISPLAY
    set_rendercolor(renderer, 0, false);
    SDL_RenderClear(renderer);

    const int visible_height = machine.visible_height();
    const int horizontal_visible_tstates = static_cast<int>(this->horizontal_visible_tstates());

    for (int y = 0; y < visible_height; ++y) {
        const std::size_t line_offset =
            static_cast<std::size_t>(y) * static_cast<std::size_t>(horizontal_visible_tstates);
        for (int bucket = 0; bucket < horizontal_visible_tstates; ++bucket) {
            int x0 = 0;
            int x1 = 0;

            if (bucket < machine.horizontal_border_left_tstates) {
                x0 = (bucket * machine.border_left) / machine.horizontal_border_left_tstates;
                x1 = ((bucket + 1) * machine.border_left) / machine.horizontal_border_left_tstates;
            } else if (bucket < machine.horizontal_border_left_tstates + machine.contention_visible_tstates) {
                const int display_bucket = bucket - machine.horizontal_border_left_tstates;
                x0 = machine.border_left + (display_bucket * machine.screen_width) / machine.contention_visible_tstates;
                x1 = machine.border_left +
                     ((display_bucket + 1) * machine.screen_width) / machine.contention_visible_tstates;
            } else {
                const int right_bucket = bucket - static_cast<int>(machine.horizontal_border_left_tstates +
                                                                   machine.contention_visible_tstates);
                x0 = machine.border_left + machine.screen_width +
                     (right_bucket * machine.border_left) / machine.horizontal_border_right_tstates;
                x1 = machine.border_left + machine.screen_width +
                     ((right_bucket + 1) * machine.border_left) / machine.horizontal_border_right_tstates;
            }

            if (x1 <= x0) {
                continue;
            }

            set_rendercolor(renderer, border_timeline[line_offset + bucket], false);
            SDL_RenderDrawLine(renderer, x0, y, x1 - 1, y);
        }
    }

    const std::size_t bytes_per_line = static_cast<std::size_t>(machine.screen_width / machine.attr_cell_size);
    for (int y = 0; y < machine.screen_height; y++) {
        const std::size_t line_offset = static_cast<std::size_t>(y) * bytes_per_line;
        for (int x = 0; x < machine.screen_width; x += machine.attr_cell_size) {
            uint8_t color = screen_attr_snapshot[line_offset + static_cast<std::size_t>(x >> 3)];
            bool flash = get_bit(color, 7);
            bool bright = get_bit(color, 6);
            uint8_t paper_color = (color >> 3) & 0x07;
            uint8_t ink_color = color & 0x07;

            if ((y & 0x7) == 0 && (x & 0x7) == 0) {
                SDL_Rect rect = {x + machine.border_left, y + machine.border_top, machine.attr_cell_size,
                                 machine.attr_cell_size};
                set_rendercolor(renderer, ((flash & invert) ? ink_color : paper_color), bright);
                SDL_RenderFillRect(renderer, &rect);
            }

            set_rendercolor(renderer, ((flash & invert) ? paper_color : ink_color), bright);
            uint8_t pixels = screen_bitmap_snapshot[line_offset + static_cast<std::size_t>(x >> 3)];
            if (pixels != 0) {
                if (pixels == 255) {
                    SDL_RenderDrawLine(renderer, x + machine.border_left, y + machine.border_top,
                                       x + machine.border_left + (machine.attr_cell_size - 1), y + machine.border_top);
                } else {
                    for (int p = 0; p < machine.attr_cell_size; p++) {
                        if (get_bit(pixels, 7 - p)) {
                            SDL_RenderDrawPoint(renderer, x + p + machine.border_left, y + machine.border_top);
                        }
                    }
                }
            }
        }
    }

    SDL_RenderPresent(renderer);
#endif
}

void ULA::clock(bool &do_exit, bool &do_break) {
    if (perf_freq == 0) {
        perf_freq = SDL_GetPerformanceFrequency();
        next_frame_deadline = SDL_GetPerformanceCounter() + (perf_freq / machine.frame_rate_hz);
    }

    const uint64_t frame_pos = counter % machine.frame_tstates;
    record_border_tstate(frame_pos);
    record_screen_tstate(frame_pos);

    // Frame-start side effects are keyed off the wrapped frame position rather
    // than the raw counter so they stay aligned across counter rollover.
    if (frame_pos == machine.frame_tstates - 1) {
        SDL_PumpEvents();
        const uint8_t *key_state = static_cast<const uint8_t *>(SDL_GetKeyboardState(NULL));

        if (key_state[SDL_SCANCODE_TAB]) {
            do_break = true;
        } else if (key_state[SDL_SCANCODE_ESCAPE]) {
            do_exit = true;
        }

        // Trigger interupt on Z80
        _z80.interrupt = true;
    } else if (frame_pos == machine.interrupt_hold_tstates - 1) {
        // Turn off interrupt
        _z80.interrupt = false;
    }

    if (frame_pos == machine.frame_tstates - 1) {
        render_frame();

        frame_counter++;
        if (frame_counter % 16 == 0) {
            if (invert)
                invert = false;
            else
                invert = true;
        }

        if (!fast_mode) {
            uint64_t now = SDL_GetPerformanceCounter();
            if (now < next_frame_deadline) {
                uint64_t remaining = next_frame_deadline - now;
                uint32_t ms = static_cast<uint32_t>((remaining * 1000) / perf_freq);
                if (ms > 0) {
                    SDL_Delay(ms);
                }
                // Busy-wait the remainder for finer granularity
                while (SDL_GetPerformanceCounter() < next_frame_deadline) {
                }
            }
            next_frame_deadline += (perf_freq / machine.frame_rate_hz);
        }
    }

    counter++;
}
