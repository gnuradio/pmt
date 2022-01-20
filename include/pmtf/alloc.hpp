#include <pmtf/pmtf_generated.h>

/*! 
 * Class used to allocate flatbuffer storage.
*/
class AlignedAllocator : public flatbuffers::Allocator {
  public:
    uint8_t* allocate(size_t size) FLATBUFFERS_OVERRIDE {
        return reinterpret_cast<uint8_t*>(std::aligned_alloc(64, size));
    }

    void deallocate(uint8_t *p, size_t) FLATBUFFERS_OVERRIDE { free(p); }
    static void dealloc(void *p, size_t) { free(p); }
};

