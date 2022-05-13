
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "instructions.h"

namespace NJVM {
    static constexpr instruction_info_t INSTRUCTION_DATA[] = {
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

            {"pushg", true},
            {"popg",  true},

            {"asf",   true},
            {"rsf",   false},
            {"pushl", true},
            {"popl",  true},

            {"eq",    false},
            {"ne",    false},
            {"lt",    false},
            {"le",    false},
            {"gt",    false},
            {"ge",    false},

            {"jmp",   true},
            {"brf",   true},
            {"brt",   true},
    };
    static constexpr opcode_t max_opcode = (sizeof(INSTRUCTION_DATA) / sizeof(instruction_info_t)) - 1;


    [[nodiscard]] const instruction_info_t &info_for_opcode(opcode_t opcode) {
        if (opcode > max_opcode) {
            std::stringstream explanation;
            explanation << "Invalid opcode " << static_cast<int>(opcode) << " is out of range.";
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


    // constexpr function to check if two strings are equal.
    static constexpr bool strequals(const char *s1, const char *s2) {
        for (size_t offset = 0; s1[offset] == s2[offset]; offset++) {
            if (s1[offset] == '\0') return true;
        }
        return false;
    }

    [[nodiscard]] constexpr opcode_t opcode_for(const char *name) {
        for (opcode_t opcode = 0; opcode <= max_opcode; opcode++) {
            if (strequals(name, INSTRUCTION_DATA[opcode].name)) {
                return opcode;
            }
        }
        throw std::invalid_argument("Unknown instruction mnemonic.");
    }


    void print_instruction(instruction_t instruction) {
        const instruction_info_t &info = info_for_opcode(get_opcode(instruction));
        std::cout << info.name;
        if (info.requires_operand) {
            std::cout << " " << get_immediate(instruction);
        }
        std::cout << std::endl;
    }


#define DO_ARITHMETIC(op) {                            \
    stack.at(sp - 2) = stack[sp - 2] op stack[sp - 1]; \
    sp--;                                              \
}

#define DO_COMPARISON(op) {                                      \
    stack.at(sp - 2) = (stack[sp - 2] op stack[sp - 1]) ? 1 : 0; \
    sp--;                                                        \
}

    bool exec_instruction(instruction_t instruction) {
        switch (get_opcode(instruction)) {
            case opcode_for("halt"):
                return false;

            case opcode_for("pushc"):
                stack.at(sp++) = get_immediate(instruction);
                break;

            case opcode_for("add"): DO_ARITHMETIC(+)
                break;

            case opcode_for("sub"): DO_ARITHMETIC(-)
                break;

            case opcode_for("mul"): DO_ARITHMETIC(*)
                break;

            case opcode_for("div"): DO_ARITHMETIC(/)
                break;

            case opcode_for("mod"): DO_ARITHMETIC(%)
                break;


            case opcode_for("rdint"):
                std::cin >> stack.at(sp++);
                break;

            case opcode_for("wrint"):
                std::cout << stack.at(--sp);
                break;

            case opcode_for("rdchr"):
                std::cin >> reinterpret_cast<char &>(stack.at(sp++));
                break;

            case opcode_for("wrchr"):
                std::cout << static_cast<char>(stack.at(--sp));
                break;


            case opcode_for("pushg"):
                stack.at(sp++) = static_data.at(get_immediate(instruction));
                break;

            case opcode_for("popg"):
                static_data.at(get_immediate(instruction)) = stack.at(--sp);
                break;

            case opcode_for("asf"): {
                int32_t size = get_immediate(instruction);
                if (size < 0) throw std::invalid_argument("Frame size can't be negative.");
                if (sp + 1 + size > stack.size()) throw std::overflow_error("Unable to allocate stack frame.");

                stack[sp++] = fp;
                fp = sp;
                sp += size;
                break;
            }

            case opcode_for("rsf"):
                sp = fp;
                fp = stack.at(--sp);
                break;

            case opcode_for("pushl"):
                stack.at(sp++) = stack.at(fp + get_immediate(instruction));
                break;

            case opcode_for("popl"):
                stack.at(fp + get_immediate(instruction)) = stack.at(--sp);
                break;


            case opcode_for("eq"): DO_COMPARISON(==)
                break;

            case opcode_for("ne"): DO_COMPARISON(!=)
                break;

            case opcode_for("lt"): DO_COMPARISON(<)
                break;

            case opcode_for("le"): DO_COMPARISON(<=)
                break;

            case opcode_for("gt"): DO_COMPARISON(>)
                break;

            case opcode_for("ge"): DO_COMPARISON(>=)
                break;


            case opcode_for("jmp"):
                pc = get_immediate(instruction);
                break;

            case opcode_for("brf"):
                if (stack[--sp] == 0) pc = get_immediate(instruction);
                break;

            case opcode_for("brt"):
                if (stack[--sp] == 1) pc = get_immediate(instruction);
                break;

            default: {
                std::stringstream ss;
                ss << "Opcode " << static_cast<int>(get_opcode(instruction))
                   << " does not reference a known instruction.";
                throw std::invalid_argument(ss.str());
            }
        }
        return true;
    }
}


