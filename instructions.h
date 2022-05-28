
#pragma once

/**
 * Definitions of supported instructions, auxiliary data, and their semantics.
 */

#include <cstdint>
#include "types.h"

namespace NJVM {

    /**
     * Struct holding the name of an instruction as well as a boolean flag
     * encoding whether an operand is encoded as part of the instruction.
     */
    struct instruction_info_t {
        const char *name;
        bool requires_operand;
    };

    /**
     * Fetches the information about an instruction for a given opcode.
     */
    [[nodiscard]] const instruction_info_t &info_for_opcode(opcode_t opcode);

    /**
     * Fetches the opcode for a given instruction mnemonic.
     *
     * This function is a constexpr function and can be evaluated during
     * compile time.
     */
    [[nodiscard]] constexpr opcode_t opcode_for(const char *name);


    /**
     * Extracts the opcode from an instruction.
     */
    [[nodiscard]] constexpr opcode_t get_opcode(instruction_t instruction);

    /**
     * Extracts the immediate value (operand) from an instruction.
     */
    [[nodiscard]] constexpr immediate_t get_immediate(instruction_t instruction);


    /**
     * Prints a human readable representation of the given instruction to the
     * standard output.
     */
    void print_instruction(instruction_t instruction);

    /**
     * Executes the given instruction.
     *
     * @return false, if the end of a program is reached, true otherwise.
     */
    bool exec_instruction(instruction_t instruction);

}
