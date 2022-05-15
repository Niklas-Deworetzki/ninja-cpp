
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

            {"call",  true},
            {"ret",   false},
            {"drop",  true},
            {"pushr", false},
            {"popr",  false},

            {"dup",   false},
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


    static stack_slot &pop() {
        return stack[--sp];
    }

    static stack_slot &push() {
        return stack[sp++];
    }

    static ObjRef new_integer(int32_t i) {
        ObjRef result = halloc(sizeof(ninja_int_t));
        *result = i;
        return result;
    }

    template<typename Binary>
    void do_arithmetic() {
        Binary op;

        ObjRef rhs = pop().as_reference();
        ObjRef lhs = pop().as_reference();

        push() = new_integer(op(*lhs, *rhs));
    }

    template<typename Comparison>
    void do_comparison() {
        Comparison op;

        ObjRef rhs = pop().as_reference();
        ObjRef lhs = pop().as_reference();

        push() = new_integer(op(*lhs, *rhs) ? 1 : 0);
    }

    bool exec_instruction(instruction_t instruction) {
        switch (get_opcode(instruction)) {
            case opcode_for("halt"):
                return false;

            case opcode_for("pushc"):
                push() = new_integer(get_immediate(instruction));
                break;

            case opcode_for("add"):
                do_arithmetic<std::plus<ninja_int_t>>();
                break;

            case opcode_for("sub"):
                do_arithmetic<std::minus<ninja_int_t>>();
                break;

            case opcode_for("mul"):
                do_arithmetic<std::multiplies<ninja_int_t>>();
                break;

            case opcode_for("div"):
                do_arithmetic<std::divides<ninja_int_t>>();
                break;

            case opcode_for("mod"):
                do_arithmetic<std::modulus<ninja_int_t>>();
                break;


            case opcode_for("rdint"):  {
                int32_t  input;
                std::cin >> input;
                push() = new_integer(input);
                break;
            }

            case opcode_for("wrint"):
                std::cout << *pop().as_reference();
                break;

            case opcode_for("rdchr"): {
                int32_t input;
                std::cin >> reinterpret_cast<char &>(input);
                push() = new_integer(input);
                break;
            }

            case opcode_for("wrchr"):
                std::cout << static_cast<char>(*pop().as_reference());
                break;


            case opcode_for("pushg"):
                push() = static_data.at(get_immediate(instruction));
                break;

            case opcode_for("popg"):
                static_data.at(get_immediate(instruction)) = pop().as_reference();
                break;

            case opcode_for("asf"): {
                int32_t size = get_immediate(instruction);
                if (size < 0) throw std::invalid_argument("Frame size can't be negative.");
                if (static_cast<uint32_t>(sp + 1 + size) > stack.size())
                    throw std::overflow_error("Unable to allocate stack frame.");

                push() = fp;
                fp = sp;
                while (size--) { // Initialize stack frame.
                    push() = nullptr;
                }
                break;
            }

            case opcode_for("rsf"):
                sp = fp;
                fp = pop().as_primitive();
                break;

            case opcode_for("pushl"):
                push() = stack.at(fp + get_immediate(instruction)).as_reference();
                break;

            case opcode_for("popl"):
                stack.at(fp + get_immediate(instruction)) = pop().as_reference();
                break;


            case opcode_for("eq"):
                do_comparison<std::equal_to<ninja_int_t>>();
                break;

            case opcode_for("ne"):
                do_comparison<std::not_equal_to<ninja_int_t>>();
                break;

            case opcode_for("lt"):
                do_comparison<std::less<ninja_int_t>>();
                break;

            case opcode_for("le"):
                do_comparison<std::less_equal<ninja_int_t>>();
                break;

            case opcode_for("gt"):
                do_comparison<std::greater<ninja_int_t>>();
                break;

            case opcode_for("ge"):
                do_comparison<std::greater_equal<ninja_int_t>>();
                break;


            case opcode_for("jmp"):
                pc = get_immediate(instruction);
                break;

            case opcode_for("brf"):
                if (*pop().as_reference() == 0) pc = get_immediate(instruction);
                break;

            case opcode_for("brt"):
                if (*pop().as_reference() == 1) pc = get_immediate(instruction);
                break;


            case opcode_for("call"):
                push() = pc;
                pc = get_immediate(instruction);
                break;

            case opcode_for("ret"):
                pc = pop().as_primitive();
                break;

            case opcode_for("drop"): // TODO: Check for negative size.
                sp -= get_immediate(instruction);
                if (sp < 0) throw std::underflow_error("Not enough elements on the stack for drop.");
                break;

            case opcode_for("pushr"):
                push() = ret;
                break;

            case opcode_for("popr"):
                ret = pop().as_reference();
                break;


            case opcode_for("dup"): {
                ObjRef duplicated = stack.at(sp - 1).as_reference();
                push() = duplicated;
                break;
            }


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


