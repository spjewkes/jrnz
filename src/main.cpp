#include <iostream>
#include <cstdlib>

#include "registers.hpp"

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv)
{
	std::cout << "Running jrnz..." << std::endl;

	if(argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <rom image>" << std::endl;
		return EXIT_FAILURE;
	}

	std::string rom_file(argv[1]);
	Z80 state(65536, rom_file);

	while(state.clock())
	{
	}

	return EXIT_SUCCESS;
}
