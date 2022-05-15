
#pragma once

#include <cstdint>
#include <stdexcept>

namespace NJVM {

    typedef uint32_t instruction_t;
    typedef uint8_t opcode_t;
    typedef int32_t immediate_t;

    typedef int32_t ninja_int_t;
    typedef ninja_int_t *ObjRef;

    struct stack_slot {
        bool isObjRef;
        union {
            ObjRef reference;
            int32_t primitive;
        } u;

        explicit stack_slot();

        stack_slot &operator=(ObjRef reference);

        stack_slot &operator=(int32_t primitive);

        [[nodiscard]] ObjRef &as_reference();

        [[nodiscard]] int32_t &as_primitive();
    };

}

