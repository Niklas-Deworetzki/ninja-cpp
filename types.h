
#pragma once

#include <cstdint>
#include <stdexcept>

namespace NJVM {

    typedef uint32_t instruction_t;
    typedef uint8_t opcode_t;
    typedef int32_t immediate_t;

    extern const size_t MAXIMUM_OBJECT_SIZE;

    struct ninja_object {
        uint32_t size;
        unsigned char data[];

        [[nodiscard]] uint32_t get_size() const;

        void mark_copied(size_t forward_reference);

        [[nodiscard]] bool is_copied() const;

        [[nodiscard]] bool is_complex() const;
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


    [[nodiscard]] ObjRef newPrimitiveObject(size_t byte_count);

    [[nodiscard]] ObjRef newCompoundObject(size_t member_count);


    template<typename numerical>
    [[nodiscard]] ObjRef &get_member(ObjRef obj, numerical index);

    template<typename numerical>
    [[nodiscard]] ObjRef newNinjaObject(numerical size);

    template<typename numeric>
    [[nodiscard]] ObjRef newNinjaInteger(numeric i);

}

