
#pragma once

#include "njvm.h"
#include "types.h"

namespace NJVM {

    class gc_config {
    public:
        size_t heap_size_kbytes;
        bool gcstats;
        bool gcpurge;
    };


    void initialize_heap(gc_config config);

    void gc();

    [[nodiscard]] ObjRef halloc(size_t payload_size);

}