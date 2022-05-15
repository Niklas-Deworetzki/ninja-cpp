
#include "types.h"

namespace NJVM {

    ObjRef nil = nullptr;


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


    [[nodiscard]] ObjRef halloc(size_t payload_size) {
        auto result = static_cast<ObjRef>(malloc(sizeof(ninja_object) + payload_size));
        if (result == nullptr) {
            throw std::bad_alloc();
        }
        return result;
    }

    ObjRef newPrimitiveObject(size_t byte_count) {
        ObjRef result = halloc(byte_count * sizeof(unsigned char));
        result->size = byte_count;
        return result;
    }

    ObjRef newCompoundObject(size_t member_count) {
        ObjRef result = halloc(member_count * sizeof(ObjRef));
        result->size = member_count;
        return result;
    }
}

