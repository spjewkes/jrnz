#include <iostream>
#include <fstream>
#include <cstdlib>

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

	if(std::ifstream rom{argv[1], std::ios::in | std::ios::binary})
	{
		rom.close();
	}
	else
	{
		std::cerr << "No file found called " << argv[1] << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
