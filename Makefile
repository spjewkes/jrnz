run_jrnz: main.cpp memory.hpp registers.hpp instructions.hpp instructions.cpp
	g++ -std=c++11 main.cpp instructions.cpp -o run_jrnz
