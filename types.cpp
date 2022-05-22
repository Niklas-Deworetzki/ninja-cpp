
#include "types.h"
#include "gc.h"

namespace NJVM {

    ObjRef nil = nullptr;

    const size_t MAXIMUM_OBJECT_SIZE = (1 << 30) - 1;
    const uint32_t COMPLEX_FLAG = 1 << 31, COPIED_FLAG = 1 << 30;


    void ninja_object::mark_copied() {
        this->size |= COPIED_FLAG;
    }

    bool ninja_object::is_copied() const {
        return (this->size & COPIED_FLAG) != 0;
    }

    bool ninja_object::is_complex() const {
        return (this->size & COMPLEX_FLAG) != 0;
    }

    uint32_t ninja_object::get_size() const {
        return this->size & ~(COMPLEX_FLAG | COPIED_FLAG);
    }


    stack_slot::stack_slot() : isObjRef(false) {
        this->u.primitive = 0;
    }

    stack_slot &stack_slot::operator=(ObjRef reference) {
        this->isObjRef = true;
        this->u.reference = reference;
        return *this;
    }

    stack_slot &stack_slot::operator=(int32_t primitive) {
        this->isObjRef = false;
        this->u.primitive = primitive;
        return *this;
    }

    [[nodiscard]] ObjRef &stack_slot::as_reference() {
        if (!this->isObjRef) throw std::logic_error("Stack slot is not a reference.");
        return u.reference;
    }

    [[nodiscard]] int32_t &stack_slot::as_primitive() {
        if (this->isObjRef) throw std::logic_error("Stack slot is a reference.");
        return u.primitive;
    }


    ObjRef newPrimitiveObject(size_t byte_count) {
        ObjRef result = halloc(byte_count * sizeof(unsigned char));
        result->size = byte_count;
        return result;
    }

    ObjRef newCompoundObject(size_t member_count) {
        ObjRef result = halloc(member_count * sizeof(ObjRef));
        result->size = member_count | COMPLEX_FLAG;
        return result;
    }
}

