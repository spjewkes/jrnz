#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>

#include "z80.hpp"
#include "ula.hpp"
#include "bus.hpp"
#include "system.hpp"
#include "debugger.hpp"
#include "options.hpp"

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

void wait_keypress()
{
	SDL_Event event;

	while(1)
	{
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT)
		{
			return;
		}
	}
}

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv)
{
	std::cout << "Running jrnz..." << std::endl;

#ifdef HAVE_DISPLAY
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

	window = SDL_CreateWindow("JRNZ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  960, 768, SDL_WINDOW_MAXIMIZED);
	if (!window)
	{
		std::cerr << "Could not create window: " << SDL_GetError() <<	std::endl;
		return EXIT_FAILURE;
    }
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetScale(renderer, 3.0f, 3.0f);
#else
    if (SDL_Init(SDL_INIT_EVENTS) != 0)
	{
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
#endif

	Options options(argc, argv);

	Bus mem(65536);
	Z80 state(mem, options.fast_mode);
	ULA ula(state, mem, options.fast_mode);
	Debugger debug(state, mem);

	System sys(state, ula, mem, debug);

// #if 0
	// Use options to set up system
	if (options.rom_on)
	{
		mem.load_rom(options.rom_file);
	}
	if (options.sna_on)
	{
		mem.load_snapshot(options.sna_file, state);
	}

	debug.set_dout(options.debug_mode);
	if (options.break_on)
	{
		debug.set_break(true, options.break_addr);
	}

	bool running = true;

	do
	{
		running = sys.clock();
	} while (running);

	std::cout << "Closing jrnz.\n";

	if (options.pause_on_quit)
	{
		std::cout << "Emulation stopped. Close window to exit.\n";
		wait_keypress();
	}
// #endif

#ifdef HAVE_DISPLAY
	SDL_DestroyWindow(window);
#endif
	SDL_Quit();
	
#if 0
	typedef struct test_data
	{
		uint8_t op1;
		uint8_t op2;
		uint8_t carry_in;
		uint8_t result;
		uint8_t carry_out;
		uint8_t overflow;
	} test_data;

	test_data adc_tests[] =
		{
			{0,   0, 0,   0, 0, 0},
			{0,   1, 0,   1, 0, 0},
			{0, 127, 0, 127, 0, 0},
			{0, 128, 0, 128, 0, 0},
			{0, 129, 0, 129, 0, 0},
			{0, 255, 0, 255, 0, 0},
			{1,   0, 0,   1, 0, 0},
			{1,   1, 0,   2, 0, 0},
			{1, 127, 0, 128, 0, 1},
			{1, 128, 0, 129, 0, 0},
			{1, 129, 0, 130, 0, 0},
			{1, 255, 0,   0, 1, 0},
			{127,   0, 0, 127, 0, 0},
			{127,   1, 0, 128, 0, 1},
			{127, 127, 0, 254, 0, 1},
			{127, 128, 0, 255, 0, 0},
			{127, 129, 0,   0, 1, 0},
			{127, 255, 0, 126, 1, 0},
			{128,   0, 0, 128, 0, 0},
			{128,   1, 0, 129, 0, 0},
			{128, 127, 0, 255, 0, 0},
			{128, 128, 0,   0, 1, 1},
			{128, 129, 0,   1, 1, 1},
			{128, 255, 0, 127, 1, 1},
			{129,   0, 0, 129, 0, 0},
			{129,   1, 0, 130, 0, 0},
			{129, 127, 0,   0, 1, 0},
			{129, 128, 0,   1, 1, 1},
			{129, 129, 0,   2, 1, 1},
			{129, 255, 0, 128, 1, 0},
			{255,   0, 0, 255, 0, 0},
			{255,   1, 0,   0, 1, 0},
			{255, 127, 0, 126, 1, 0},
			{255, 128, 0, 127, 1, 1},
			{255, 129, 0, 128, 1, 0},
			{255, 255, 0, 254, 1, 0},
			{0,   0, 1,   1, 0, 0},
			{0,   1, 1,   2, 0, 0},
			{0, 127, 1, 128, 0, 1},
			{0, 128, 1, 129, 0, 0},
			{0, 129, 1, 130, 0, 0},
			{0, 255, 1,   0, 1, 0},
			{1,   0, 1,   2, 0, 0},
			{1,   1, 1,   3, 0, 0},
			{1, 127, 1, 129, 0, 1},
			{1, 128, 1, 130, 0, 0},
			{1, 129, 1, 131, 0, 0},
			{1, 255, 1,   1, 1, 0},
			{127,   0, 1, 128, 0, 1},
			{127,   1, 1, 129, 0, 1},
			{127, 127, 1, 255, 0, 1},
			{127, 128, 1,   0, 1, 0},
			{127, 129, 1,   1, 1, 0},
			{127, 255, 1, 127, 1, 0},
			{128,   0, 1, 129, 0, 0},
			{128,   1, 1, 130, 0, 0},
			{128, 127, 1,   0, 1, 0},
			{128, 128, 1,   1, 1, 1},
			{128, 129, 1,   2, 1, 1},
			{128, 255, 1, 128, 1, 0},
			{129,   0, 1, 130, 0, 0},
			{129,   1, 1, 131, 0, 0},
			{129, 127, 1,   1, 1, 0},
			{129, 128, 1,   2, 1, 1},
			{129, 129, 1,   3, 1, 1},
			{129, 255, 1, 129, 1, 0},
			{255,   0, 1,   0, 1, 0},
			{255,   1, 1,   1, 1, 0},
			{255, 127, 1, 127, 1, 0},
			{255, 128, 1, 128, 1, 0},
			{255, 129, 1, 129, 1, 0},
			{255, 255, 1, 255, 1, 0},
		};

	size_t length = sizeof(adc_tests) / sizeof(adc_tests[0]);
	for (size_t i=0; i<length; i++)
	{
		state.af.flags(0);
		uint8_t result = adc_tests[i].op1;
		StorageElement dst = StorageElement(&result, 1);
		StorageElement src = StorageElement(adc_tests[i].op2);
		if (adc_tests[i].carry_in)
		{
			state.af.flag(RegisterAF::Flags::Carry, true);
		}

		std::cout << "Adding " << src << " to " << dst << " (and carry " << static_cast<uint32_t>(adc_tests[i].carry_in) << ") ";

		Instruction instruction = Instruction(InstType::ADC, "test", 0, 0);
		instruction.do_adc(state, dst, src);

		std::cout << "gives: " << dst << std::endl;

		if (result != adc_tests[i].result)
		{
			std::cerr << "  ERROR: result was : " << dst << " expected: " << result << std::endl;
		}

		bool carry_out = (adc_tests[i].carry_out ? true : false);
		bool overflow = (adc_tests[i].overflow ? true : false);

		if (state.af.flag(RegisterAF::Flags::Carry) != carry_out)
		{
			std::cerr << "  ERROR: carry was: " << state.af.flag(RegisterAF::Flags::Carry) << " expected: " << carry_out << std::endl;
		}

		if (state.af.flag(RegisterAF::Flags::ParityOverflow) != overflow)
		{
			std::cerr << "  ERROR: overflow was: " << state.af.flag(RegisterAF::Flags::ParityOverflow) << " expected: " << overflow << std::endl;
		}
	}
#endif

#if 0
	test_data adc_tests[] =		{
  0(   0) -   0(   0) - 0 =   0(   0) CY=0 OV=0
  0(   0) -   1(   1) - 0 = 255(  -1) CY=1 OV=0
  0(   0) - 127( 127) - 0 = 129(-127) CY=1 OV=0
  0(   0) - 128(-128) - 0 = 128(-128) CY=1 OV=1
  0(   0) - 129(-127) - 0 = 127( 127) CY=1 OV=0
  0(   0) - 255(  -1) - 0 =   1(   1) CY=1 OV=0
  1(   1) -   0(   0) - 0 =   1(   1) CY=0 OV=0
  1(   1) -   1(   1) - 0 =   0(   0) CY=0 OV=0
  1(   1) - 127( 127) - 0 = 130(-126) CY=1 OV=0
  1(   1) - 128(-128) - 0 = 129(-127) CY=1 OV=1
  1(   1) - 129(-127) - 0 = 128(-128) CY=1 OV=1
  1(   1) - 255(  -1) - 0 =   2(   2) CY=1 OV=0
127( 127) -   0(   0) - 0 = 127( 127) CY=0 OV=0
127( 127) -   1(   1) - 0 = 126( 126) CY=0 OV=0
127( 127) - 127( 127) - 0 =   0(   0) CY=0 OV=0
127( 127) - 128(-128) - 0 = 255(  -1) CY=1 OV=1
127( 127) - 129(-127) - 0 = 254(  -2) CY=1 OV=1
127( 127) - 255(  -1) - 0 = 128(-128) CY=1 OV=1
128(-128) -   0(   0) - 0 = 128(-128) CY=0 OV=0
128(-128) -   1(   1) - 0 = 127( 127) CY=0 OV=1
128(-128) - 127( 127) - 0 =   1(   1) CY=0 OV=1
128(-128) - 128(-128) - 0 =   0(   0) CY=0 OV=0
128(-128) - 129(-127) - 0 = 255(  -1) CY=1 OV=0
128(-128) - 255(  -1) - 0 = 129(-127) CY=1 OV=0
129(-127) -   0(   0) - 0 = 129(-127) CY=0 OV=0
129(-127) -   1(   1) - 0 = 128(-128) CY=0 OV=0
129(-127) - 127( 127) - 0 =   2(   2) CY=0 OV=1
129(-127) - 128(-128) - 0 =   1(   1) CY=0 OV=0
129(-127) - 129(-127) - 0 =   0(   0) CY=0 OV=0
129(-127) - 255(  -1) - 0 = 130(-126) CY=1 OV=0
255(  -1) -   0(   0) - 0 = 255(  -1) CY=0 OV=0
255(  -1) -   1(   1) - 0 = 254(  -2) CY=0 OV=0
255(  -1) - 127( 127) - 0 = 128(-128) CY=0 OV=0
255(  -1) - 128(-128) - 0 = 127( 127) CY=0 OV=0
255(  -1) - 129(-127) - 0 = 126( 126) CY=0 OV=0
255(  -1) - 255(  -1) - 0 =   0(   0) CY=0 OV=0
  0(   0) -   0(   0) - 1 = 255(  -1) CY=1 OV=0
  0(   0) -   1(   1) - 1 = 254(  -2) CY=1 OV=0
  0(   0) - 127( 127) - 1 = 128(-128) CY=1 OV=0
  0(   0) - 128(-128) - 1 = 127( 127) CY=1 OV=0
  0(   0) - 129(-127) - 1 = 126( 126) CY=1 OV=0
  0(   0) - 255(  -1) - 1 =   0(   0) CY=1 OV=0
  1(   1) -   0(   0) - 1 =   0(   0) CY=0 OV=0
  1(   1) -   1(   1) - 1 = 255(  -1) CY=1 OV=0
  1(   1) - 127( 127) - 1 = 129(-127) CY=1 OV=0
  1(   1) - 128(-128) - 1 = 128(-128) CY=1 OV=1
  1(   1) - 129(-127) - 1 = 127( 127) CY=1 OV=0
  1(   1) - 255(  -1) - 1 =   1(   1) CY=1 OV=0
127( 127) -   0(   0) - 1 = 126( 126) CY=0 OV=0
127( 127) -   1(   1) - 1 = 125( 125) CY=0 OV=0
127( 127) - 127( 127) - 1 = 255(  -1) CY=1 OV=0
127( 127) - 128(-128) - 1 = 254(  -2) CY=1 OV=1
127( 127) - 129(-127) - 1 = 253(  -3) CY=1 OV=1
127( 127) - 255(  -1) - 1 = 127( 127) CY=1 OV=0
128(-128) -   0(   0) - 1 = 127( 127) CY=0 OV=1
128(-128) -   1(   1) - 1 = 126( 126) CY=0 OV=1
128(-128) - 127( 127) - 1 =   0(   0) CY=0 OV=1
128(-128) - 128(-128) - 1 = 255(  -1) CY=1 OV=0
128(-128) - 129(-127) - 1 = 254(  -2) CY=1 OV=0
128(-128) - 255(  -1) - 1 = 128(-128) CY=1 OV=0
129(-127) -   0(   0) - 1 = 128(-128) CY=0 OV=0
129(-127) -   1(   1) - 1 = 127( 127) CY=0 OV=1
129(-127) - 127( 127) - 1 =   1(   1) CY=0 OV=1
129(-127) - 128(-128) - 1 =   0(   0) CY=0 OV=0
129(-127) - 129(-127) - 1 = 255(  -1) CY=1 OV=0
129(-127) - 255(  -1) - 1 = 129(-127) CY=1 OV=0
255(  -1) -   0(   0) - 1 = 254(  -2) CY=0 OV=0
255(  -1) -   1(   1) - 1 = 253(  -3) CY=0 OV=0
255(  -1) - 127( 127) - 1 = 127( 127) CY=0 OV=1
255(  -1) - 128(-128) - 1 = 126( 126) CY=0 OV=0
255(  -1) - 129(-127) - 1 = 125( 125) CY=0 OV=0
255(  -1) - 255(  -1) - 1 = 255(  -1) CY=1 OV=0
		};
#endif

	return EXIT_SUCCESS;
}
