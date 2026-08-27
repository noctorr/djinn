#include "VulkanAlloc.hpp"
#include <cstdlib>
#include <cstring>

#ifdef __VK_GCC_ATTR
[[gnu::hot]] [[gnu::malloc]]
#endif
void* Djinn_VKMemoryAlloc::VKHeap_Buffer::heap_allocCallback(
    void* pUserdata,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope allocScope
) {
    switch (size) {
        case 0uz:
        return NULL;
        default:
        return std::aligned_alloc(alignment, size);
    }
}

#ifdef __VK_GCC_ATTR
[[gnu::hot]] [[gnu::malloc]]
#endif
void* Djinn_VKMemoryAlloc::VKHeap_Buffer::heap_reallocCallback(
    void* pUserdata,
    void* pOriginal,
    size_t size,
    size_t allignment,
    VkSystemAllocationScope allocScope
) {
    if ( !pOriginal || memalignment(pOriginal) != allignment ) {
        return NULL;
    } else {
        void* buffer = std::aligned_alloc(allignment, size);

        std::memcpy(buffer, pOriginal, size);
        std::free(pOriginal);

        return buffer;
    }
}

#ifdef __VK_GCC_ATTR
[[gnu::hot]]
#endif
void Djinn_VKMemoryAlloc::VKHeap_Buffer::heap_freeCallback(
    void* pUserdata,
    void* pMemory
) {
    if ( pMemory ) {
        std::free(pMemory);
    }
}