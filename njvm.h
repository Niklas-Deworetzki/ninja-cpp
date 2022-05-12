
#pragma once

#include <vector>
#include <stack>

namespace NJVM {

    constexpr uint32_t version = 2;

    extern const char *MESSAGE_START, *MESSAGE_STOP;

    typedef uint32_t instruction_t;

    typedef uint8_t opcode_t;
    typedef int32_t immediate_t;

    typedef int32_t ninja_int_t;

    extern std::vector<instruction_t> program;
    extern std::vector<ninja_int_t> stack, static_data;
    extern int32_t pc, sp, fp;

}

