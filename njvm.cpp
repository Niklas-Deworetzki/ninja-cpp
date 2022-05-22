#include <iostream>
#include <algorithm>
#include <cstring>

#include "njvm.h"
#include "instructions.h"
#include "loader.h"
#include "gc.h"

namespace NJVM {
    // Definition of NJVM constants and registers.
    const char *MESSAGE_START = "Ninja Virtual Machine started";
    const char *MESSAGE_STOP = "Ninja Virtual Machine stopped";

    std::vector<instruction_t> program;
    std::vector<ObjRef> static_data;
    std::vector<stack_slot> stack;
    int32_t pc = 0, sp = 0, fp = 0;
    ObjRef ret = nullptr;
}


struct cli_config {
    bool requested_version = false;
    bool requested_help = false;
    bool requested_list = false;
    size_t stack_size_kbytes = 64;
    NJVM::gc_config gc_config = {8192, false, false};
    char *input_file = nullptr;
};

static cli_config parse_arguments(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    cli_config config = parse_arguments(argc, argv);

    if (config.input_file == nullptr) {
        config.requested_help = true; // Input file is required.
        std::cout << "No input file given!" << std::endl;
    }

    if (config.requested_version || config.requested_help) {
        std::cout << "NJVM version " << NJVM::version << " (build date " << __DATE__ << ") ";
        std::cout << "Copyright Niklas Deworetzki" << std::endl;
    }
    if (config.requested_help) {
        // TODO
    }
    if (config.requested_help || config.requested_version) {
        return 0; // Interrupt execution.
    }

    NJVM::load(config.input_file);
    using namespace NJVM;

    if (config.requested_list) {
        for (const auto &instruction: program) {
            print_instruction(instruction);
        }

    } else {
        // Translate config number into 1024 bytes and allocate stack slots accordingly.
        stack = std::vector<stack_slot>(config.stack_size_kbytes * 1024 / sizeof(stack_slot));
        initialize_heap(config.gc_config);

        std::cout << MESSAGE_START << std::endl;
        {
            instruction_t instruction;
            do {
                instruction = program.at(pc);        // Fetch instruction.
                pc++;                                // Increment pc.
            } while (exec_instruction(instruction)); // Execute instruction.
        }
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
        char *arg = argv[i];

        if (arg[0] == '-' && !encountered_separator) {
            if (matches(arg, {"--version", "-v"})) {
                config.requested_version = true;

            } else if (matches(arg, {"--help", "-h"})) {
                config.requested_help = true;

            } else if (matches(arg, {"--list"})) {
                config.requested_list = true;

            } else if (matches(arg, {"--gcpurge"})) {
                config.gc_config.gcpurge = true;

            } else if (matches(arg, {"--gcstats"})) {
                config.gc_config.gcstats = true;

            } else if (matches(arg, {"--stack"})) {
                if (argc > i + 1) {
                    config.stack_size_kbytes = std::stoi(argv[i + 1]);
                } else {
                    throw std::invalid_argument("Missing argument to --stack flag.");
                }

            } else if (matches(arg, {"--heap"})) {
                if (argc > i + 1) {
                    config.gc_config.heap_size_kbytes = std::stoi(argv[i + 1]);
                } else {
                    throw std::invalid_argument("Missing argument to --heap flag.");
                }


            } else if (matches(arg, {"--"})) {
                encountered_separator = true;

            } else {
                throw std::invalid_argument(
                        std::string("Unknown argument `").append(arg).append("' encountered."));
            }

        } else {
            config.input_file = arg;
        }

    }
    return config;
}
