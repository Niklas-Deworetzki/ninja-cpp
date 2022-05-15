
#pragma once

#include <vector>
#include <stack>

#include "types.h"

namespace NJVM {

    constexpr uint32_t version = 7;

    extern const char *MESSAGE_START, *MESSAGE_STOP;

    extern std::vector<instruction_t> program;
    extern std::vector<ObjRef> static_data;
    extern std::vector<stack_slot> stack;
    extern int32_t pc, sp, fp;
    extern ObjRef ret;

}

