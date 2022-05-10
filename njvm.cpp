#include <iostream>
#include <algorithm>
#include <cstring>

#include "njvm.h"

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


struct cli_config {
    bool requested_version = false;
    bool requested_help = false;
};

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
}

int main(int argc, char *argv[]) {
    cli_config config = parse_arguments(argc, argv);

    std::cout << NJVM::MESSAGE_START << std::endl;
    std::cout << NJVM::MESSAGE_STOP << std::endl;

    return 0;
}
