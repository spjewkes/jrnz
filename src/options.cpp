/**
 * @brief Source file implementing the programm options class.
 */

#include <iostream>
#include <getopt.h>

#include "options.hpp"

void Options::print_help()
{
    std::cout << "Run: " << m_argv[0] << "[--help] [--debug] [--rom <filename>] [--break <line_no>]\n";
    std::cout << "\t--help            - displays this help\n";
    std::cout << "\t--rom <filename>  - Loads the specified ROM file (at address 0)\n";
	std::cout << "\t--sna <filename>  - Loads the specified SNA file into memory (at address 16384)\n";
    std::cout << "\t--debug           - switched on debug output\n";
	std::cout << "\t--break <line_no> - Enable breakpoint at the specified line number\n";
	exit(EXIT_SUCCESS);
}

void Options::process()
{
    static struct option long_options[] =
        {
            { "help",    no_argument,       0, 'h' },
            { "debug",   no_argument,       0, 'd' },
            { "rom",     required_argument, 0, 'r' },
			{ "break",   required_argument, 0, 'b' },
			{ "sna",     required_argument, 0, 's' },
            { 0, 0, 0, 0 }
        };

    int c;

    while (1)
    {
        int option_index = 0;

        c = getopt_long(m_argc, m_argv, "hdr:s:", long_options, &option_index);

        if (c == -1)
        {
            break;
        }

        switch (c)
		{
        case 'h':
		{
            print_help();
            break;
		}

		case 'd':
		{
            debug_mode = true;
            break;
		}

		case 'r':
		{
            rom_file = optarg;
			rom_on = true;
            break;
		}

		case 's':
		{
			sna_file = optarg;
			sna_on = true;
			break;
		}

		case 'b':
		{
			unsigned long int val = strtoul(optarg, NULL, 0);
			if (val > UINT16_MAX)
			{
				std::cerr << "Break line should not exceed a 16-bit address\n";
				abort();
			}

			break_addr = static_cast<uint16_t>(val);
			break_on = true;
			break;
		}
		
		}

    }
}
