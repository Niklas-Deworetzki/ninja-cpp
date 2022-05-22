
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cstring>

#include "instructions.h"

extern "C" {
#include "lib/bigint.h"
}

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

            {"new",   true},
            {"getf",  true},
            {"putf",  true},

            {"newa",  false},
            {"getfa", false},
            {"putfa", false},
            {"getsz", false},

            {"pushn", false},
            {"refeq", false},
            {"refne", false},
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

    template<typename numeric>
    static ObjRef new_integer(numeric i) {
        bigFromInt(static_cast<int>(i));
        return reinterpret_cast<ObjRef>(bip.res);
    }

    template<void Binary()>
    static void do_arithmetic() {
        bip.op2 = pop().as_reference();
        bip.op1 = pop().as_reference();
        Binary();
    }

    template<typename Comparator>
    static ObjRef do_comparison() {
        bip.op2 = pop().as_reference();
        bip.op1 = pop().as_reference();

        Comparator cmp;
        return new_integer(cmp(bigCmp(), 0));
    }

    template<typename numerical>
    static ObjRef &get_member(ObjRef obj, numerical index) {
        if (index < 0 || static_cast<size_t>(index) >= obj->size) {
            std::stringstream buffer;
            buffer << "Cannot access member #" << index << " on object of size " << obj->size << ".";
            throw std::range_error(buffer.str());
        }
        return reinterpret_cast<ObjRef *>(obj->data)[index];
    }

    template<typename numerical>
    static ObjRef create_of_size(numerical size) {
        if (size < 0) {
            throw std::logic_error("Cannot create object of negative size.");
        }
        ObjRef created = newCompoundObject(size);
        while (size--) { // Initialize all members with nil.
            reinterpret_cast<ObjRef *>(created->data)[size] = nil;
        }
        return created;
    }

    bool exec_instruction(instruction_t instruction) {
        switch (get_opcode(instruction)) {
            case opcode_for("halt"):
                return false;

            case opcode_for("pushc"):
                push() = new_integer(get_immediate(instruction));
                break;

            case opcode_for("add"):
                do_arithmetic<bigAdd>();
                push() = reinterpret_cast<ObjRef>(bip.res);
                break;

            case opcode_for("sub"):
                do_arithmetic<bigSub>();
                push() = reinterpret_cast<ObjRef>(bip.res);
                break;

            case opcode_for("mul"):
                do_arithmetic<bigMul>();
                push() = reinterpret_cast<ObjRef>(bip.res);
                break;

            case opcode_for("div"):
                do_arithmetic<bigDiv>();
                push() = reinterpret_cast<ObjRef>(bip.res);
                break;

            case opcode_for("mod"):
                do_arithmetic<bigDiv>();
                push() = reinterpret_cast<ObjRef>(bip.rem);
                break;


            case opcode_for("rdint"): {
                bigRead(stdin);
                push() = reinterpret_cast<ObjRef>(bip.res);
                break;
            }

            case opcode_for("wrint"):
                bip.op1 = pop().as_reference();
                bigPrint(stdout);
                break;

            case opcode_for("rdchr"): {
                int32_t input;
                std::cin >> reinterpret_cast<char &>(input);
                push() = new_integer(input);
                break;
            }

            case opcode_for("wrchr"):
                bip.op1 = pop().as_reference();
                std::cout << static_cast<char>(bigToInt());
                break;


            case opcode_for("pushg"):
                push() = static_data.at(get_immediate(instruction));
                break;

            case opcode_for("popg"):
                static_data.at(get_immediate(instruction)) = pop().as_reference();
                break;

            case opcode_for("asf"): {
                immediate_t size = get_immediate(instruction);
                if (size < 0) throw std::invalid_argument("Frame size can't be negative.");
                if (static_cast<uint32_t>(sp + 1 + size) > stack.size())
                    throw std::overflow_error("Unable to allocate stack frame.");

                push() = fp;
                fp = sp;
                while (size--) { // Initialize stack frame.
                    push() = nil;
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
                push() = do_comparison<std::equal_to<int>>();
                break;

            case opcode_for("ne"):
                push() = do_comparison<std::not_equal_to<int>>();
                break;

            case opcode_for("lt"):
                push() = do_comparison<std::less<int>>();
                break;

            case opcode_for("le"):
                push() = do_comparison<std::less_equal<int>>();
                break;

            case opcode_for("gt"):
                push() = do_comparison<std::greater<int>>();
                break;

            case opcode_for("ge"):
                push() = do_comparison<std::greater_equal<int>>();
                break;


            case opcode_for("jmp"):
                pc = get_immediate(instruction);
                break;

            case opcode_for("brf"):
                bip.op1 = pop().as_reference();
                if (bigToInt() == 0) pc = get_immediate(instruction);
                break;

            case opcode_for("brt"):
                bip.op1 = pop().as_reference();
                if (bigToInt() != 0) pc = get_immediate(instruction);
                break;


            case opcode_for("call"):
                push() = pc;
                pc = get_immediate(instruction);
                break;

            case opcode_for("ret"):
                pc = pop().as_primitive();
                break;

            case opcode_for("drop"): {
                immediate_t size = get_immediate(instruction);
                if (size < 0) throw std::invalid_argument("Frame size can't be negative.");
                if (static_cast<uint32_t>(size) > stack.size())
                    throw std::overflow_error("Not enough elements on the stack for drop.");

                sp -= size;
                break;
            }

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


            case opcode_for("new"): {
                push() = create_of_size(get_immediate(instruction));
                break;
            }

            case opcode_for("getf"): {
                ObjRef object = pop().as_reference();
                immediate_t member = get_immediate(instruction);

                push() = get_member(object, member);
                break;
            }

            case opcode_for("putf"): {
                ObjRef value = pop().as_reference();
                ObjRef object = pop().as_reference();
                immediate_t member = get_immediate(instruction);

                get_member(object, member) = value;
                break;
            }

            case opcode_for("newa"): {
                bip.op1 = pop().as_reference();

                push() = create_of_size(bigToInt());
                break;
            }

            case opcode_for("getfa"): {
                bip.op1 = pop().as_reference();
                ObjRef array = pop().as_reference();

                push() = get_member(array, bigToInt());
                break;
            }

            case opcode_for("putfa"): {
                ObjRef value = pop().as_reference();
                bip.op1 = pop().as_reference();
                ObjRef array = pop().as_reference();

                get_member(array, bigToInt()) = value;
                break;
            }

            case opcode_for("getsz"):
                push() = new_integer(pop().as_reference()->size);
                break;


            case opcode_for("pushn"):
                push() = nil;
                break;

            case opcode_for("refeq"): {
                bool result = pop().as_reference() == pop().as_reference();
                push() = new_integer(result);
                break;
            }

            case opcode_for("refne"): {
                bool result = pop().as_reference() != pop().as_reference();
                push() = new_integer(result);
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


