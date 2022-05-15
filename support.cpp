
#include <stdexcept>
#include "njvm.h"

extern "C" {
#include "lib/support.h"
}


void fatalError(char *msg) {
    throw std::logic_error(msg);
}

void *newPrimObject(int dataSize) {
    return NJVM::halloc(dataSize);
}

void *getPrimObjectDataPointer(void *primObject) {
    return reinterpret_cast<NJVM::ninja_object*>(primObject)->data;
}
