
#include <iostream>
#include "gc.h"


namespace NJVM {
    bool gcstats, gcpurge;
    char *heap = nullptr;

    char *active_half, *backup_half;
    size_t available_size, bytes_used = 0;

    size_t allocations = 0;

    void initialize_heap(gc_config config) {
        if (heap != nullptr) {
            throw std::logic_error("Heap already initialized!");
        }

        gcstats = config.gcstats;
        gcpurge = config.gcpurge;

        available_size = (config.heap_size_kbytes * 1024) / 2;
        heap = static_cast<char *>(malloc(available_size * 2));
        if (heap == nullptr) {
            throw std::bad_alloc();
        }

        active_half = heap;
        backup_half = heap + available_size;
    }


    void gc() {
        if (gcstats) {
            std::cout << "Allocated since last gc: " << allocations << " objects (" << bytes_used << " bytes)."
                      << std::endl;
        }
    }


    [[nodiscard]] ObjRef halloc(size_t payload_size) {
        auto result = static_cast<ObjRef>(malloc(sizeof(ninja_object) + payload_size));
        if (result == nullptr) {
            throw std::bad_alloc();
        }
        return result;
    }

}