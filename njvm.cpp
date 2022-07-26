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

    // Leave components default-initialized for now.
    std::vector<instruction_t> program;
    std::vector<ObjRef> static_data;
    std::vector<stack_slot> stack;

    // Initialize registers.
    int32_t pc = 0, sp = 0, fp = 0;
    ObjRef ret = nil;
}


struct cli_config {
    bool requested_version = false;
    bool requested_help = false;
    bool requested_list = false;
    size_t stack_size_kbytes = NJVM::DEFAULT_STACK_SIZE;
    NJVM::gc_config gc_config = {
            .heap_size_kbytes = NJVM::DEFAULT_HEAP_SIZE,
            .gcstats = false,
            .gcpurge = false,
    };
    char *input_file = nullptr;
};

/**
 * Function used to parse cli parameters. Returns a populated cli_config struct.
 */
static cli_config parse_arguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    try {
        cli_config config = parse_arguments(argc, argv);

        if (config.input_file == nullptr) {
            config.requested_help = true; // Input file is required.
            std::cout << "No input file given!" << std::endl;
        }

        if (config.requested_version) {
            std::cout << "NJVM version " << NJVM::version << " (build date " << __DATE__ << ")";
        }
        if (config.requested_help) {
            std::cout << "Usage: " << argv[0] << " INPUT [FLAG...]\n";
            std::cout << "Execute Ninja binary files in a virtual machine.\n\n";
            std::cout << " --help\n";
            std::cout << "              Display this help page.\n";
            std::cout << " --version\n";
            std::cout << "              Prints the supported binary version number. This machine\n";
            std::cout << "              can execute binary files created by an assembler with\n";
            std::cout << "              an equal or lower version.\n";
            std::cout << " --list\n";
            std::cout << "              Print a listing of the loaded program and exit. No\n";
            std::cout << "              instructions will be executed.\n";
            std::cout << " --stack SIZE\n";
            std::cout << "              Sets the size of this machine's stack to SIZE kilobytes.\n";
            std::cout << "              Default is " << NJVM::DEFAULT_STACK_SIZE << "\n";
            std::cout << " --heap SIZE\n";
            std::cout << "              Sets the size of this machine's heap to SIZE kilobytes.\n";
            std::cout << "              Default is " << NJVM::DEFAULT_HEAP_SIZE << "\n";
            std::cout << " --gcpurge\n";
            std::cout << "              Purge memory after garbage collection. This will erase\n";
            std::cout << "              all remains of collected objects.\n";
            std::cout << " --gcstats\n";
            std::cout << "              Display statistics with every garbage collection run.\n";
            std::cout << std::endl;
        }
        if (config.requested_help || config.requested_version) {
            std::cout << "Copyright (C) Niklas Deworetzki 2022" << std::endl;
            return 0; // Interrupt execution.
        }

        using namespace NJVM;
        load(config.input_file); // Load program, initializing program and static_data.

        if (config.requested_list) {
            for (const auto &instruction: program) {
                print_instruction(instruction);
            }

        } else {
            const size_t stack_slot_count = (config.stack_size_kbytes * 1024) / sizeof(stack_slot);
            // Initialize stack and heap for execution.
            stack = std::vector<stack_slot>(stack_slot_count);
            initialize_heap(config.gc_config);

            std::cout << MESSAGE_START << std::endl;
            {
                instruction_t instruction;
                do {
                    instruction = program.at(pc);        // Fetch instruction.
                    pc++;                                // Increment pc.
                } while (exec_instruction(instruction)); // Execute instruction.
            }
            gc(); // Perform gc at end of execution to force it on small programs.
            std::cout << MESSAGE_STOP << std::endl;
        }

        // Free up memory.
        program.clear();
        static_data.clear();
        free_heap();

        return 0;
    } catch (std::exception &exception) {
        std::cerr << exception.what();
        return 1;
    }
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
    cli_config config{}; // Default-initialize config struct.
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
                    config.stack_size_kbytes = std::stoul(argv[i + 1]);
                    i++;
                } else {
                    throw std::invalid_argument("Missing argument to --stack flag.");
                }

            } else if (matches(arg, {"--heap"})) {
                if (argc > i + 1) {
                    config.gc_config.heap_size_kbytes = std::stoul(argv[i + 1]);
                    i++;
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
