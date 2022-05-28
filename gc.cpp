
#include <iostream>
#include <cstring>

#include "gc.h"
#include "njvm.h"


namespace NJVM {
    // Global heap management configuration.
    bool gcstats, gcpurge;
    unsigned char *heap = nullptr;

    // Heap is split into two halfs.
    /**
     * Pointer to active heap half.
     */
    unsigned char *active_half,
    /**
     * Pointer to secondary heap half.
     * Used to copy live objects during garbage collection.
     */
    *unused_half;

    /**
     * Amount of bytes available per heap half.
     *
     * This value is not touched after initialization.
     */
    size_t bytes_available,
    /**
     * Amount of bytes used in the currently active heap half.
     */
    bytes_used = 0;

    /**
     * Amount of allocations in the currently active heap half.
     */
    size_t allocations = 0;

    void initialize_heap(gc_config config) {
        if (heap != nullptr) {
            throw std::logic_error("Heap already initialized!");
        }

        gcstats = config.gcstats;
        gcpurge = config.gcpurge;

        const size_t total_heap_size = config.heap_size_kbytes * 1024;
        if (total_heap_size > 2 * MAXIMUM_HEAP_HALF_SIZE) {
            std::stringstream ss;
            ss << "Requested heap size of " << total_heap_size << " bytes"
               << " exceeds limit of " << (2 * MAXIMUM_HEAP_HALF_SIZE) << " bytes.";
            throw std::logic_error(ss.str());
        }

        bytes_available = total_heap_size / 2;
        heap = static_cast<unsigned char *>(malloc(total_heap_size));
        if (heap == nullptr) {
            throw std::bad_alloc();
        }

        active_half = heap;
        unused_half = heap + bytes_available;
        if (gcpurge) {
            std::memset(heap, 0, total_heap_size);
        }
    }

    void free_heap() {
        free(heap);
    }


    /**
     * Allocate the given amount of bytes on the active heap half.
     *
     * This function does not perform any checks. It is only used to
     * keep management information consistent.
     */
    [[nodiscard]] static inline ObjRef allocate(size_t size) {
        unsigned char *allocated = active_half + bytes_used;
        bytes_used += size;
        allocations++;
        return reinterpret_cast<ObjRef>(allocated);
    }

    /**
     * Rescues the object referenced by the given parameter. The storage location is passed as a pointer
     * so the reference can be updated to point to the copy allocated on the other heap half.
     */
    static void rescue(ObjRef *original) { /* ObjRef& would be nicer but doesn't work well with bip registers. */
        ObjRef &originalReference = *original;

        if (originalReference == nil) {
            // Object reference is nil reference. This value is unchanged.

        } else if (originalReference->is_copied()) {
            // Referenced object was already copied. Update reference.
            originalReference = reinterpret_cast<ObjRef>(active_half + (*original)->get_size());

        } else {
            // Allocate a copy.
            ObjRef copied = allocate(object_size(originalReference->get_size(), originalReference->is_compound()));
            copied->tag = originalReference->tag; // Copy size including flags.

            // Mark original as copied and place forward reference.
            originalReference->mark_copied(reinterpret_cast<unsigned char *>(copied) - active_half);

            // Rescue all members, if this element stores references.
            if (copied->is_compound()) {
                for (size_t i = 0; i < copied->get_size(); i++) {
                    rescue(&get_member(originalReference, i));
                }

            }
            // Actually copy data from original object.
            std::memcpy(copied->data, originalReference->data, payload_size(copied->get_size(), copied->is_compound()));
            originalReference = copied; // Update reference to refer to copy.
        }
    }

    void gc() {
        if (gcstats) {
            std::cerr << "Allocated since last gc: " << allocations << " objects (" << bytes_used << " bytes)."
                      << std::endl;
        }
        // Reset management information.
        bytes_used = 0;
        allocations = 0;

        // Mark the other half active as it is now used to allocate objects during copying.
        std::swap(active_half, unused_half);

        // Rescue objects stored in bip registers.
        rescue(reinterpret_cast<ObjRef *>(&bip.op1));
        rescue(reinterpret_cast<ObjRef *>(&bip.op2));
        rescue(reinterpret_cast<ObjRef *>(&bip.res));
        rescue(reinterpret_cast<ObjRef *>(&bip.rem));
        // Rescue objects stored in return register.
        rescue(&ret);
        // Rescue objects stored in static data.
        for (auto &entry: static_data) {
            rescue(&entry);
        }
        // Rescue objects stored on stack.
        for (int32_t offset = 0; offset < sp; offset++) {
            if (stack[offset].isObjRef) {
                rescue(&stack[offset].as_reference());
            }
        }

        if (gcstats) {
            std::cerr << "Live objects: " << allocations << " (" << bytes_used << " bytes)."
                      << std::endl;
            std::cerr << (bytes_available - bytes_used) << " bytes are available for use."
                      << std::endl;
        }
        if (gcpurge) {
            std::memset(unused_half, 0, bytes_available);
        }
    }


    [[nodiscard]] ObjRef halloc(size_t size) {
        if (size < object_size(0, false)) {
            throw std::invalid_argument("Cannot allocate object with less than zero members.");
        }

        if (size > MAXIMUM_OBJECT_SIZE || size > bytes_available) {
            std::stringstream ss;
            ss << "Requested object of size " << size << " exceeds limits of heap.";
            throw std::invalid_argument(ss.str());
        }

        if (size > (bytes_available - bytes_used)) {
            // Not enough heap space available. Try to reclaim using garbage collection.
            gc();
            if (size > (bytes_available - bytes_used)) {
                // Still not enough heap space available, but no more space can be reclaimed.
                throw std::runtime_error("Out of memory.");
            }
        }

        return allocate(size);
    }
}