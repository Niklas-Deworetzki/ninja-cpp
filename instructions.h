
#pragma once

#include <cstdint>
#include "types.h"

namespace NJVM {

    struct instruction_info_t {
        const char *name;
        bool requires_operand;
    };

    [[nodiscard]] const instruction_info_t &info_for_opcode(opcode_t opcode);

    [[nodiscard]] constexpr opcode_t opcode_for(const char *name);


    [[nodiscard]] constexpr opcode_t get_opcode(instruction_t instruction);

    [[nodiscard]] constexpr immediate_t get_immediate(instruction_t instruction);


    void print_instruction(instruction_t instruction);

    bool exec_instruction(instruction_t instruction);

}
