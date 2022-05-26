
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
        uint32_t instruction_count;
        uint32_t static_vars_count;
    };

    void load(const char *filename) {
        FILE *input = fopen(filename, "rb");

        if (input == nullptr) {
            std::stringstream ss;
            ss << "Unable to open file " << filename << ": " << std::strerror(errno);
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


        NJVM::program = std::vector<instruction_t>(header.instruction_count);
        NJVM::static_data = std::vector<ObjRef>(header.static_vars_count);
        for (auto &entry: static_data) {
            entry = nil;
        }


        if (fread(NJVM::program.data(), sizeof(instruction_t), header.instruction_count, input) !=
            header.instruction_count) {
            throw std::invalid_argument("Failed to read program from input file.");
        }
    }
}
