
#include <cstdint>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sstream>

#include "njvm.h"
#include "loader.h"

namespace NJVM {

    struct NJVM_file_header {
        char magic[4];
        uint32_t version;
        uint32_t n_instruction;
        uint32_t n_static_vars;
    };

    void load(const char *filename) {
        FILE *input = fopen(filename, "rb");

        if (input == nullptr) {
            std::stringstream ss;
            ss << "Unable to open file: " << std::strerror(errno);
            throw std::invalid_argument(ss.str());
        }

        NJVM_file_header header;
        if (fread(&header, sizeof(header), 1, input) != 1) {
            throw std::invalid_argument("Failed to read header from input file.");
        }

        if (strncmp(header.magic, "NJBF", 4) != 0) {
            throw std::invalid_argument("Invalid header in input file.");
        }

        if (header.version > NJVM::version) {
            throw std::invalid_argument("Unsupported binary version.");
        }


        NJVM::program = std::vector<instruction_t>(header.n_instruction);
        NJVM::static_data = std::vector<ObjRef>(header.n_static_vars); // TODO: Initialize with nullptr.

        if (fread(NJVM::program.data(), sizeof(instruction_t), header.n_instruction, input) != header.n_instruction) {
            throw std::invalid_argument("Failed to read program from input file.");
        }
    }
}
