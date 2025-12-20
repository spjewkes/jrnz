/**
 * Brief Implementation of the ULA class.
 */

#include "ula.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>

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

void ULA::clock(bool &do_exit, bool &do_break) {
    if (perf_freq == 0) {
        perf_freq = SDL_GetPerformanceFrequency();
        next_frame_deadline = SDL_GetPerformanceCounter() + (perf_freq / 50);
    }

    switch (counter) {
        case 0: {
            SDL_PumpEvents();
            const uint8_t *key_state = static_cast<const uint8_t *>(SDL_GetKeyboardState(NULL));

            if (key_state[SDL_SCANCODE_TAB]) {
                do_break = true;
            } else if (key_state[SDL_SCANCODE_ESCAPE]) {
                do_exit = true;
            }
        }

            // Trigger interupt on Z80
            _z80.interrupt = true;
            break;

        case 32:
            // Turn off interrupt
            _z80.interrupt = false;
            break;

        case 69888:
            //! TODO - a bit of a hack, but every 50th of a second (running at 3.5
            //! Mhz on a 48K)
            //! reset the counter to start everything again.
            counter = UINT64_MAX;  // will wrap on increment

#ifdef HAVE_DISPLAY
            {
                // The draw routine at the moment is not very sophisticated and will not
                // show any clever tricks with changing attributes midway through the
                // frame. This will need and overhaul at some point in the future but is
                // sufficient for the time being.

                // Clear screen
                set_rendercolor(renderer, _bus.port_254 & 0x7, false);
                SDL_RenderClear(renderer);

                uint8_t *data = &_bus[0x4000];
                uint8_t *col_data = &_bus[0x5800];
                for (int y = 0; y < 192; y++) {
                    int new_y = 0xc0 & y;
                    new_y |= (y & 0x7) << 3;
                    new_y |= (y >> 3) & 0x7;
                    int col_y = new_y >> 3;
                    for (int x = 0; x < 256; x += 8) {
                        uint8_t color = col_data[col_y * 32 + (x >> 3)];
                        bool flash = get_bit(color, 7);
                        bool bright = get_bit(color, 6);
                        uint8_t paper_color = (color >> 3) & 0x07;
                        uint8_t ink_color = color & 0x07;

                        // Draw paper color as 8x8 pixel block
                        if ((new_y & 0x7) == 0 && (x & 0x7) == 0) {
                            SDL_Rect rect = {x + 32, new_y + 32, 8, 8};
                            set_rendercolor(renderer, ((flash & invert) ? ink_color : paper_color), bright);
                            SDL_RenderFillRect(renderer, &rect);
                        }

                        // Draw horizontal byte in ink color
                        set_rendercolor(renderer, ((flash & invert) ? paper_color : ink_color), bright);
                        uint8_t pixels = *data;
                        if (pixels != 0) {
                            if (pixels == 255) {
                                SDL_RenderDrawLine(renderer, x + 32, new_y + 32, x + 39, new_y + 32);
                            } else {
                                for (int p = 0; p < 8; p++) {
                                    if (get_bit(*data, 7 - p)) {
                                        SDL_RenderDrawPoint(renderer, x + p + 32, new_y + 32);
                                    }
                                }
                            }
                        }
                        data++;
                    }
                }

                SDL_RenderPresent(renderer);
            }
#endif

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
                next_frame_deadline += (perf_freq / 50);
            }
    }

    counter++;
}
