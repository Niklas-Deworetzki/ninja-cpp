
#pragma once

/**
 * Garbage collection and heap implementation for NJVM.
 */

#include "types.h"

namespace NJVM {

    /**
     * Garbage collection and heap configuration.
     */
    struct gc_config {
        size_t heap_size_kbytes;
        bool gcstats;
        bool gcpurge;
    };


    /**
     * Initialize heap manager and garbage collection with the given configuration.
     */
    void initialize_heap(gc_config config);

    /**
     * Frees up the memory allocated in the heap.
     */
    void free_heap();

    /**
     * Perform garbage collection.
     */
    void gc();

    /**
     * Allocate an Object with the given amount of bytes on the heap.
     */
    [[nodiscard]] ObjRef halloc(size_t size);

}