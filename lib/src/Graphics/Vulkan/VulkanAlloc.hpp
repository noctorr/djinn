#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>

#if defined(__GCC__) || defined(__clang__)
#define __VK_GCC_ATTR
#endif

namespace Djinn_VKMemoryAlloc {
    struct VKHeap_Buffer final
    {
        #ifdef __VK_GCC_ATTR
        [[gnu::hot]] [[gnu::malloc]]
        #endif
        void* heap_allocCallback(
            void* pUserdata,
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocScope
        );

        #ifdef __VK_GCC_ATTR
        [[gnu::hot]] [[gnu::malloc]]
        #endif
        void* heap_reallocCallback(
            void* pUserdata,
            void* pOriginal,
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocScope
        );

        #ifdef __VK_GCC_ATTR
        [[gnu::hot]]
        #endif
        void heap_freeCallback(
            void* pUserdata,
            void* pMemory
        );

        #ifdef __VK_GCC_ATTR
        [[gnu::hot]]
        #endif
        void heap_internalAllocCallback(
            void* pUserdata,
            size_t size,
            VkInternalAllocationType allocType,
            VkSystemAllocationScope allocScope
        );
    };
}