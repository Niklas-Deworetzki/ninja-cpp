#include <iostream>
#include <algorithm>
#include <cstring>

#include "njvm.h"
#include "instructions.h"

#define VERSION 1.0

namespace NJVM {
    const char *MESSAGE_START = "Ninja Virtual Machine started";
    const char *MESSAGE_STOP = "Ninja Virtual Machine stopped";

    std::vector<instruction_t> program;
    std::vector<ninja_int_t> stack;
    int32_t pc = 0;
    int32_t sp = 0;
}

struct cli_config {
    bool requested_version = false;
    bool requested_help = false;
    bool requested_list = false;
};

static cli_config parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    cli_config config = parse_arguments(argc, argv);

    if (config.requested_version || config.requested_help) {
        std::cout << "NJVM version " << VERSION << " (" << __DATE__ << ")" << std::endl;
        std::cout << "(C) Niklas Deworetzki" << std::endl;
    }
    if (config.requested_help) {
        // TODO
    }
    if (config.requested_help || config.requested_version) {
        return 0; // Interrupt execution.
    }

    using namespace NJVM;

    if (config.requested_list) {
        for (const auto &instruction: program) {
            NJVM::print_instruction(instruction);
        }

    } else {
        std::cout << MESSAGE_START << std::endl;

        instruction_t instruction;
        do {
            instruction = program.at(pc);        // Fetch instruction.
            pc++;                                // Increment pc.
        } while (exec_instruction(instruction)); // Execute instruction.

        std::cout << MESSAGE_STOP << std::endl;
    }

    return 0;
}


/**
 * A function used to check whether a C-style string matches a set of other
 * C-style strings. The set of expected strings are passed as std::initializer_list,
 * allowing us to specify multiple matching candidates.
 */
static bool matches(const char *arg, std::initializer_list<const char *> potential_matches) {
    return std::ranges::any_of(potential_matches, [&arg](const char *element) {
        return strcmp(arg, element) == 0;
    });
}

static cli_config parse_arguments(int argc, char *argv[]) {
    cli_config config{};
    bool encountered_separator = false;

    for (int i = 1 /* Skip program name */; i < argc; i++) {
        const char *arg = argv[i];

        if (arg[0] == '-' && !encountered_separator) {
            if (matches(arg, {"--version", "-v"})) {
                config.requested_version = true;

            } else if (matches(arg, {"--help", "-h"})) {
                config.requested_help = true;

            } else if (matches(arg, {"--list"})) {
                config.requested_list = true;

            } else if (matches(arg, {"--"})) {
                encountered_separator = true;

            } else {
                std::string explanation;
                explanation.append("Unknown argument `").append(arg).append("' encountered.");
                throw std::invalid_argument(explanation);
            }

        } else {
            // Filename.
        }

    }
    return config;
}
