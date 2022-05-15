
#include "types.h"

namespace NJVM {

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


}

