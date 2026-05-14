#include <SDL2/SDL.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "beeper.hpp"
#include "bus.hpp"
#include "debugger.hpp"
#include "machine_config.hpp"
#include "options.hpp"
#include "system.hpp"
#include "ula.hpp"
#include "z80.hpp"

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

namespace {
MachineModel selected_machine_model(MachineFamily family) {
    switch (family) {
        case MachineFamily::Spectrum48K:
            return spectrum_48k_model();
        case MachineFamily::Spectrum128K:
            return spectrum_128k_model();
    }
    return spectrum_48k_model();
}

std::string find_default_rom_path(const MachineModel &machine) {
    namespace fs = std::filesystem;

    for (std::size_t i = 0; i < machine.default_rom_filename_count; ++i) {
        const fs::path filename = machine.default_rom_filenames[i];
        const fs::path root_candidate = filename;
        if (fs::exists(root_candidate)) {
            return root_candidate.string();
        }

        const fs::path roms_candidate = fs::path("roms") / filename;
        if (fs::exists(roms_candidate)) {
            return roms_candidate.string();
        }
    }

    return "";
}
}  // namespace

void wait_keypress() {
    SDL_Event event;

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            return;
        }
    }
}

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv) {
    std::cout << "Running jrnz..." << std::endl;
    Options options(argc, argv);
    const MachineModel machine = selected_machine_model(options.machine_family);

#ifdef HAVE_DISPLAY
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow("JRNZ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, machine.window_width(),
                              machine.window_height(), 0);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetScale(renderer, machine.render_scale, machine.render_scale);
#else
    if (SDL_Init(SDL_INIT_EVENTS) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
#endif

    Bus mem(machine);
    Z80 state(mem, options.fast_mode);
    ULA ula(machine, state, mem, options.fast_mode);
    Debugger debug(state, mem);
    Beeper beeper(machine);

    System sys(state, ula, mem, debug, beeper);

    // Use options to set up system
    std::string rom_file = options.rom_file;
    if (!options.rom_on) {
        rom_file = find_default_rom_path(machine);
    }
    if (!rom_file.empty()) {
        mem.load_rom(rom_file);
    }
    if (options.sna_on) {
        mem.load_snapshot(options.sna_file, state);
    } else if (options.z80_on) {
        mem.load_z80(options.z80_file, state);
    }

    debug.set_dout(options.debug_mode);
    if (options.break_on) {
        debug.set_break(true, options.break_addr);
    }

    bool running = true;

    do {
        running = sys.clock();
    } while (running);

    std::cout << "Closing jrnz.\n";

    if (options.pause_on_quit) {
        std::cout << "Emulation stopped. Close window to exit.\n";
        wait_keypress();
    }

#ifdef HAVE_DISPLAY
    SDL_DestroyWindow(window);
#endif
    SDL_Quit();

    return EXIT_SUCCESS;
}
