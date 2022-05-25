
#include <iostream>
#include <cstring>

#include "gc.h"
#include "njvm.h"


namespace NJVM {
    bool gcstats, gcpurge;
    char *heap = nullptr;

    char *active_half, *unused_half;
    size_t bytes_available, bytes_used = 0;

    size_t allocations = 0;

    void initialize_heap(gc_config config) {
        if (heap != nullptr) {
            throw std::logic_error("Heap already initialized!");
        }

        gcstats = config.gcstats;
        gcpurge = config.gcpurge;

        const size_t total_heap_size = config.heap_size_kbytes * 1024;
        bytes_available = total_heap_size / 2;
        heap = static_cast<char *>(malloc(total_heap_size));
        if (heap == nullptr) {
            throw std::bad_alloc();
        }

        active_half = heap;
        unused_half = heap + bytes_available;
        if (gcpurge) {
            std::memset(heap, 0, total_heap_size);
        }
    }

    [[nodiscard]] static ObjRef allocate(size_t size) {
        char *allocated = active_half + bytes_used;
        bytes_used += size;
        allocations++;
        return reinterpret_cast<ObjRef>(allocated);
    }


    [[nodiscard]] static ObjRef rescue(ObjRef original) {
        if (original == nil) {
            return nil;
        } else if (original->is_copied()) { // Already rescued. Update reference to point to copied Object.
            return reinterpret_cast<ObjRef>(active_half + original->get_size());
        } else {
            // Allocate a copy.
            ObjRef copied = allocate(object_size(original->get_size(), original->is_complex()));
            copied->size = original->size; // Copy size including flags.

            // Mark this as copied and place forward reference.
            original->mark_copied(reinterpret_cast<char *>(copied) - active_half);

            // Rescue all members, if this element stores references.
            if (copied->is_complex()) {
                for (size_t i = 0; i < copied->get_size(); i++) {
                    ObjRef &member = get_member(original, i);
                    member = rescue(member);
                }

            }
            // Actually copy data from original object.
            std::memcpy(copied->data, original->data, payload_size(copied->get_size(), copied->is_complex()));

            return copied;
        }
    }

    void gc() {
        if (gcstats) {
            std::cout << "Allocated since last gc: " << allocations << " objects (" << bytes_used << " bytes)."
                      << std::endl;
        }
        bytes_used = 0;
        allocations = 0;

        std::swap(active_half, unused_half);

        bip.op1 = rescue(reinterpret_cast<ObjRef>(bip.op1));
        bip.op2 = rescue(reinterpret_cast<ObjRef>(bip.op2));
        bip.res = rescue(reinterpret_cast<ObjRef>(bip.res));
        bip.rem = rescue(reinterpret_cast<ObjRef>(bip.rem));
        ret = rescue(ret);
        for (int32_t offset = 0; offset < sp; offset++) {
            if (stack[offset].isObjRef) {
                stack[offset] = rescue(stack[offset].as_reference());
            }
        }
        for (auto &entry: static_data) {
            entry = rescue(entry);
        }

        if (gcstats) {
            std::cout << "Live objects: " << allocations << " (" << bytes_used << " bytes)."
                      << std::endl;
            std::cout << (bytes_available - bytes_used) << " bytes are available for use."
                      << std::endl;
        }
        if (gcpurge) {
            std::memset(unused_half, 0, bytes_available);
        }
    }


    [[nodiscard]] ObjRef halloc(size_t size) {
        if (size > MAXIMUM_OBJECT_SIZE) {
            std::stringstream ss;
            ss << "Requested object size of " << size << " exceeds size limit of "
               << MAXIMUM_OBJECT_SIZE << ".";
            throw std::invalid_argument(ss.str());
        }

        if (size > (bytes_available - bytes_used)) {
            gc();
            if (size > (bytes_available - bytes_used)) {
                throw std::runtime_error("Out of memory.");
            }
        }

        return allocate(size);
    }

}