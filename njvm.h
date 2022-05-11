
#pragma once

#include <vector>

namespace NJVM {

    extern const char *MESSAGE_START, *MESSAGE_STOP;

    typedef uint32_t instruction_t;

    typedef uint8_t opcode_t;
    typedef int32_t immediate_t;

    typedef int32_t ninja_int_t;

    extern std::vector<instruction_t> program;

}

