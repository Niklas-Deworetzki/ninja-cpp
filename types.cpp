
#include "types.h"
#include "gc.h"


namespace NJVM {

    // Two most significant bits of the object tag are used to store data.
    const uint32_t COMPOUND_FLAG = 1 << 31,
            COPIED_FLAG = 1 << 30;


    //-----------------------------------------------------------------------
    // Ninja object member functions.
    //-----------------------------------------------------------------------

    void ninja_object::mark_copied(size_t forward_reference) {
        this->tag = COPIED_FLAG | forward_reference;
    }

    bool ninja_object::is_copied() const {
        return (this->tag & COPIED_FLAG) != 0;
    }

    bool ninja_object::is_compound() const {
        return (this->tag & COMPOUND_FLAG) != 0;
    }

    uint32_t ninja_object::get_size() const {
        return this->tag & ~(COMPOUND_FLAG | COPIED_FLAG); // Dont include data bits in size.
    }


    //-----------------------------------------------------------------------
    // stack slot object member functions.
    //-----------------------------------------------------------------------

    stack_slot::stack_slot() : isObjRef(false) {
        this->u.internal = 0;
    }

    stack_slot &stack_slot::operator=(ObjRef reference) {
        this->isObjRef = true;
        this->u.reference = reference;
        return *this;
    }

    stack_slot &stack_slot::operator=(int32_t primitive) {
        this->isObjRef = false;
        this->u.internal = primitive;
        return *this;
    }

    [[nodiscard]] ObjRef &stack_slot::as_reference() {
        if (!this->isObjRef) throw std::logic_error("Stack slot does not hold a reference.");
        return u.reference;
    }

    [[nodiscard]] int32_t &stack_slot::as_primitive() {
        if (this->isObjRef) throw std::logic_error("Stack slot holds a reference.");
        return u.internal;
    }


    //-----------------------------------------------------------------------
    // Allocation functions for Ninja objects.
    //-----------------------------------------------------------------------

    [[nodiscard]] ObjRef allocateIntegerObject(size_t byte_count) {
        ObjRef result = halloc(object_size(byte_count, false));
        result->tag = byte_count;
        return result;
    }

    [[nodiscard]] ObjRef allocateCompoundObject(size_t member_count) {
        ObjRef result = halloc(object_size(member_count, true));
        result->tag = member_count | COMPOUND_FLAG;
        return result;
    }
}

