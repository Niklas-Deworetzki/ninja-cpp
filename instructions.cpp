
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "instructions.h"

namespace NJVM {
    static const instruction_info_t INSTRUCTION_DATA[] = {
            {"halt",  false},

            {"pushc", true},

            {"add",   false},
            {"sub",   false},
            {"mul",   false},
            {"div",   false},
            {"mod",   false},

            {"rdint", false},
            {"wrint", false},
            {"rdchr", false},
            {"wrchr", false},
    };
    static constexpr opcode_t max_opcode = (sizeof(INSTRUCTION_DATA) / sizeof(instruction_info_t)) - 1;


    [[nodiscard]] const instruction_info_t &info_for_opcode(opcode_t opcode) {
        if (opcode > max_opcode) {
            std::stringstream explanation;
            explanation << "Invalid opcode " << opcode << " is out of range.";
            throw std::invalid_argument(explanation.str());
        }
        return INSTRUCTION_DATA[opcode];
    }


    [[nodiscard]] constexpr opcode_t get_opcode(instruction_t instruction) {
        return (instruction >> 24) & 0xFF;
    }

    [[nodiscard]] constexpr immediate_t get_immediate(instruction_t instruction) {
        auto intermediate = static_cast<immediate_t>(instruction & 0x00FFFFFF);
        if (intermediate & 0x00800000) {
            // If original immediate was negative, fill remaining bits to extend sign.
            intermediate |= (0xFF << 24);
        }
        return intermediate;
    }

    void print_instruction(instruction_t instruction) {
        const instruction_info_t &info = info_for_opcode(get_opcode(instruction));
        std::cout << info.name;
        if (info.requires_operand) {
            std::cout << " " << get_opcode(instruction);
        }
        std::cout << std::endl;
    }

    void exec_instruction(instruction_t instruction) {
        // TODO
    }


    // constexpr function to check if two strings are equal.
    static constexpr bool strequals(const char *s1, const char *s2) {
        for (size_t offset = 0; s1[offset] == s2[offset]; offset++) {
            if (s1[offset] == '\0') return true;
        }
        return false;
    }

    [[nodiscard]] constexpr opcode_t opcode_for(const char *name) {
        for (opcode_t opcode = 0; opcode < max_opcode; opcode++) {
            if (strequals(name, info_for_opcode(opcode).name)) {
                return opcode;
            }
        }
        throw std::invalid_argument("Unknown instruction mnemonic.");
    }
}


