
#pragma once

#include <cstdint>
#include <stdexcept>

namespace NJVM {

    typedef uint32_t instruction_t;
    typedef uint8_t opcode_t;
    typedef int32_t immediate_t;

    struct ninja_object {
        size_t size;
        unsigned char data[];
    };
    typedef ninja_object *ObjRef;

    extern ObjRef nil;


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


    [[nodiscard]] ObjRef halloc(size_t payload_size);

    [[nodiscard]] ObjRef newPrimitiveObject(size_t byte_count);

    [[nodiscard]] ObjRef newCompoundObject(size_t member_count);

}

