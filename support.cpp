
/**
 * Implementation of functions required for big-integer implementation.
 */

#include <stdexcept>
#include "types.h"

void fatalError(char *msg) {
    throw std::logic_error(msg);
}

void *newPrimObject(int dataSize) {
    return NJVM::allocateIntegerObject(dataSize);
}

void *getPrimObjectDataPointer(void *primObject) {
    return reinterpret_cast<NJVM::ObjRef>(primObject)->data;
}
