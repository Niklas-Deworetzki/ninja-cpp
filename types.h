
#pragma once

#include <cstdint>
#include <stdexcept>
#include <sstream>

extern "C" {
#include "lib/bigint.h"
}

namespace NJVM {

    typedef uint32_t instruction_t;
    typedef uint8_t opcode_t;
    typedef int32_t immediate_t;

    struct ninja_object {
        uint32_t size;
        unsigned char data[0];

        [[nodiscard]] uint32_t get_size() const;

        void mark_copied(size_t forward_reference);

        [[nodiscard]] bool is_copied() const;

        [[nodiscard]] bool is_complex() const;
    };

    constexpr size_t MAXIMUM_OBJECT_SIZE = (1 << 30) - 1;

    typedef ninja_object *ObjRef;

    constexpr ObjRef nil = nullptr;

    constexpr size_t object_size(size_t member_count, bool is_complex) {
        return sizeof(ninja_object) + member_count * (is_complex ? sizeof(ObjRef) : sizeof(unsigned char));
    }


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
    [[nodiscard]] ObjRef &get_member(ObjRef obj, numerical index) {
        if (index < 0 || static_cast<size_t>(index) >= obj->get_size()) {
            std::stringstream buffer;
            buffer << "Cannot access member #" << index << " on object of size " << obj->get_size() << ".";
            throw std::range_error(buffer.str());
        }
        return reinterpret_cast<ObjRef *>(obj->data)[index];
    }

    template<typename numerical>
    [[nodiscard]] ObjRef newNinjaObject(numerical size) {
        if (size < 0) {
            throw std::logic_error("Cannot create object of negative size.");
        }
        ObjRef created = newCompoundObject(size);
        while (size--) { // Initialize all members with nil.
            get_member(created, size) = nil;
        }
        return created;
    }

    template<typename numeric>
    [[nodiscard]] ObjRef newNinjaInteger(numeric i) {
        bigFromInt(static_cast<int>(i));
        return reinterpret_cast<ObjRef>(bip.res);
    }
}

