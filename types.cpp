
#include <sstream>
#include "types.h"
#include "gc.h"

extern "C" {
#include "lib/bigint.h"
}

namespace NJVM {

    ObjRef nil = nullptr;

    const size_t MAXIMUM_OBJECT_SIZE = (1 << 30) - 1;
    const uint32_t COMPLEX_FLAG = 1 << 31, COPIED_FLAG = 1 << 30;


    void ninja_object::mark_copied(size_t forward_reference) {
        this->size = COPIED_FLAG | forward_reference;
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


    [[nodiscard]] ObjRef newPrimitiveObject(size_t byte_count) {
        ObjRef result = halloc(byte_count * sizeof(unsigned char));
        result->size = byte_count;
        return result;
    }

    [[nodiscard]] ObjRef newCompoundObject(size_t member_count) {
        ObjRef result = halloc(member_count * sizeof(ObjRef));
        result->size = member_count | COMPLEX_FLAG;
        return result;
    }


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
            reinterpret_cast<ObjRef *>(created->data)[size] = nil;
        }
        return created;
    }

    template<typename numeric>
    [[nodiscard]] ObjRef newNinjaInteger(numeric i) {
        bigFromInt(static_cast<int>(i));
        return reinterpret_cast<ObjRef>(bip.res);
    }
}

