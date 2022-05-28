
#include <cstdint>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sstream>

#include "njvm.h"
#include "loader.h"

namespace NJVM {

    /**
     * Binary file header contains a magic number, a binary version,
     * the amount of instructions stored and the amount of objects
     * allocated in the static data area.
     */
    struct NJVM_file_header {
        char magic[NJBF_MAGIC_SIZE];
        uint32_t version;
        uint32_t instruction_count;
        uint32_t static_vars_count;
    };

    void load(const char *filename) {
        FILE *input = fopen(filename, "rb"); // Open the file to read binary data.

        if (input == nullptr) { // Failed to open file.
            std::stringstream ss;
            ss << "Unable to open file " << filename << ": " << std::strerror(errno);
            throw std::invalid_argument(ss.str());
        }

        NJVM_file_header header; // Allocate header and read bytes into it.
        if (fread(&header, sizeof(header), 1, input) != 1) {
            throw std::invalid_argument("Failed to read header from input file.");
        }

        if (strncmp(header.magic, NJBF_MAGIC, NJBF_MAGIC_SIZE) != 0) { // Check if header starts with magic.
            throw std::invalid_argument("Invalid header in input file.");
        }

        if (header.version > NJVM::version) { // Check if binary version is supported.
            throw std::invalid_argument("Unsupported binary version.");
        }

        // Read instruction directly into memory, after allocating enough space.
        program = std::vector<instruction_t>(header.instruction_count);
        if (fread(program.data(), sizeof(instruction_t), header.instruction_count, input) !=
            header.instruction_count) {
            throw std::invalid_argument("Failed to read program from input file.");
        }

        // Allocate static data area and initialize with nil.
        static_data = std::vector<ObjRef>(header.static_vars_count);
        for (auto &entry: static_data) {
            entry = nil;
        }
    }
}
