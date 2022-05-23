
#pragma once

/**
 * Basic definitions of the Ninja Virtual Machine.
 *
 * This header provides version number, start & stop message and the
 * machine components.
 */

#include <vector>
#include <stack>

#include "types.h"

namespace NJVM {

    constexpr uint32_t version = 8;

    extern const char *MESSAGE_START, *MESSAGE_STOP;

    constexpr size_t DEFAULT_HEAP_SIZE = 8192,
            DEFAULT_STACK_SIZE = 64;


    // Use a vector instead of raw memory. This gives us bounds checks for free.
    extern std::vector<instruction_t> program;
    extern std::vector<ObjRef> static_data;
    extern std::vector<stack_slot> stack;
    // 32-Bit integers for stack and program registers.
    extern int32_t pc, sp, fp;
    // Return register holds a reference.
    extern ObjRef ret;

}

