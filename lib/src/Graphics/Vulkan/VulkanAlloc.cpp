#include "VulkanAlloc.hpp"
#include <cstdlib>

#ifdef __VK_GCC_ATTR
[[gnu::hot]]
#endif
inline static void* Djinn_VKMemoryAlloc::heap_allocCallback(
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

