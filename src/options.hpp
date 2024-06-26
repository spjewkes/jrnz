/**
 * @brief Header file handling program options.
 */

#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Defines options class.
 */
class Options {
public:
    Options(int argc, char **argv) : m_argc(argc), m_argv(argv) { process(); }
    ~Options() {}

    std::string rom_file = {""};
    bool rom_on = {false};

    std::string sna_file = {""};
    bool sna_on = {false};

    std::string z80_file = {""};
    bool z80_on = {false};

    bool debug_mode = {false};

    uint16_t break_addr = {0};
    bool break_on = {false};

    bool fast_mode = {false};
    bool pause_on_quit = {false};

private:
    Options() = delete;

    void print_help();
    void process();

    int m_argc;
    char **m_argv;
};
