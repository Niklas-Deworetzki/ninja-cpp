
#pragma once

/**
 * Types definitions for Ninja runtime objects and functions
 * to interact with them.
 */

#include <cstdint>
#include <stdexcept>
#include <sstream>

extern "C" { // big-integer library is a C library and must be included accordingly.
#include "lib/bigint.h"
}

namespace NJVM {

    /**
     * 32-bit type representing an instruction of opcode and operand.
     */
    typedef uint32_t instruction_t;
    /**
     * 8-bit opcode used to identify an instruction variant.
     */
    typedef uint8_t opcode_t;
    /**
     * 32-bit sign-extended immediate value extracted from an instruction operand.
     */
    typedef int32_t immediate_t;

    /**
     * Ninja object definition.
     */
    struct ninja_object {
        /**
         * Tag encoding size of object including variant (integer, array/record) and
         * garbage collection data.
         */
        uint32_t tag;
        /**
         * Array holding raw object data. The actual size of this data is determined
         * at runtime and set to 0 so no extra bytes are allocated for instances of
         * this struct.
         */
        unsigned char data[0];

        /**
         * Returns the amount of values stored in the data section of this object.
         *
         * This may either represent the amount of raw bytes or the amount of
         * references to other objects.
         */
        [[nodiscard]] uint32_t get_size() const;

        /**
         * Returns true if this objects represents a compound object. Returns false
         * if this is an integer instead.
         *
         * The size of a compound object describes how many object references are
         * stored, while the size of an integer described how many raw bytes are
         * stored instead.
         */
        [[nodiscard]] bool is_compound() const;

        /**
         * Mark this object as copied during garbage collection. The new location
         * where the copy was stored is encoded within this object.
         */
        void mark_copied(size_t forward_reference);

        /**
         * Returns true, if mark_copied has been called on this object.
         */
        [[nodiscard]] bool is_copied() const;
    };

    /**
     * The largest possible size of a single object. There is no guarantee that the
     * NJVM actually allocates an object this large.
     */
    constexpr size_t MAXIMUM_OBJECT_SIZE = (UINT32_MAX >> 2) - 1;

    /**
     * The largest possible size of a single heap half. The combined byte size of all
     * live objects cannot exceed this number.
     */
    constexpr size_t MAXIMUM_HEAP_HALF_SIZE = (UINT32_MAX >> 2) - 1;


    /**
     * A type representing a reference to a Ninja object.
     */
    typedef ninja_object *ObjRef;

    /**
     * The null reference.
     */
    constexpr ObjRef nil = nullptr;


    /**
     * Compute the amount of bytes required to store a Ninja object payload of an object
     * storing the given amount of members.
     */
    constexpr size_t payload_size(size_t member_count, bool is_compound) {
        return member_count * (is_compound ? sizeof(ObjRef) : sizeof(unsigned char));
    }

    /**
     * Compute the total amount of bytes required to store a Ninja object. This includes
     * the payload as well as the object header.
     */
    constexpr size_t object_size(size_t member_count, bool is_compound) {
        return sizeof(ninja_object) + payload_size(member_count, is_compound);
    }


    /**
     * Single slot in the VM runtime stack.
     *
     * Instances of this struct may either store an object reference or
     * a primitive integer value, that is only used for internals and
     * is opaque for the executed Ninja program.
     */
    struct stack_slot {
        bool isObjRef; // Flag indicating which kind of value is stored in this slot.
        union {
            ObjRef reference;
            int32_t internal;
        } u;

        /**
         * Define an explicit constructor.
         */
        explicit stack_slot();

        /**
         * Allow assignments of object references to stack slots, updating the
         * slot's management information to indicate it holds an object reference.
         */
        stack_slot &operator=(ObjRef reference);

        /**
         * Allow assignments of primitive integer values to stack slots, updating
         * the slot's management information to indicate it holds an internal value.
         */
        stack_slot &operator=(int32_t primitive);

        /**
         * Expose the object reference stored in this stack slot.
         *
         * This function fails if an internal value is stored instead.
         */
        [[nodiscard]] ObjRef &as_reference();

        /**
         * Expose the primitive integer value stored in this stack slot.
         *
         * This function fails if an object reference is stored instead.
         */
        [[nodiscard]] int32_t &as_primitive();
    };


    /**
     * Allocate a Ninja integer object allocating the given amount of bytes as payload.
     *
     * This function will not initialize any of the data stored in the object.
     */
    [[nodiscard]] ObjRef allocateIntegerObject(size_t byte_count);

    /**
     * Allocate a Ninja compound object allocating the given amount of object
     * references as payload.
     *
     * This function will not initialize any of the data stored in the object.
     */
    [[nodiscard]] ObjRef allocateCompoundObject(size_t member_count);


    /**
     * Access the given object by index, returning a reference to the given object
     * reference stored in the object.
     * This function does not attempt to make bound checks or verify that a compound
     * object is accessed.
     *
     * @tparam numerical The type describing the index.
     * @param obj The object to access.
     * @param index The index of the member accessed on the object.
     */
    template<typename numerical>
    [[nodiscard]] ObjRef &get_member(ObjRef obj, const numerical index) noexcept {
        return reinterpret_cast<ObjRef *>(obj->data)[index];
    }

    /**
     * Access the given object by index, returning a reference to the given object
     * reference stored in the object.
     * This function verifies that the given object is a compound object and that
     * the index is valid within this object. If these restrictions are violated
     * it fails instead.
     *
     * @tparam numerical The type describing the index.
     * @param obj The object to access.
     * @param index The index of the member accessed on the object.
     */
    template<typename numerical>
    [[nodiscard]] ObjRef &try_access_member(ObjRef obj, const numerical index) {
        if (!obj->is_compound()) {
            throw std::logic_error("Cannot access members of Integer object.");
        }
        if (index < 0 || static_cast<size_t>(index) >= obj->get_size()) {
            std::stringstream buffer;
            buffer << "Cannot access member #" << index << " on object of size " << obj->get_size() << ".";
            throw std::range_error(buffer.str());
        }
        return reinterpret_cast<ObjRef *>(obj->data)[index];
    }


    /**
     * Create a new Ninja compound object with the given amount of members and
     * initialize them all to be a nil reference.
     *
     * @tparam numerical The type to describe the amount of members.
     * @param size The amount of members allocated for the object.
     */
    template<typename numerical>
    [[nodiscard]] ObjRef newNinjaObject(const numerical size) {
        if (size < 0) {
            throw std::logic_error("Cannot create object of negative size.");
        }

        ObjRef created = allocateCompoundObject(size);
        for (numerical index = 0; index < size; index++) {
            get_member(created, index) = nil; // Initialize all members with nil.
        }
        return created;
    }

    /**
     * Create a new Ninja integer object with the given numerical value.
     *
     * @tparam numeric The type to describe the integer value. It is casted to int.
     * @param i The integer value of the created Ninja object.
     */
    template<typename numeric>
    [[nodiscard]] ObjRef newNinjaInteger(const numeric i) {
        bigFromInt(static_cast<int>(i));
        return reinterpret_cast<ObjRef>(bip.res);
    }
}

