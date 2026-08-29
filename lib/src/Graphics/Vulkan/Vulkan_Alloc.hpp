#pragma once

#include "Vulkan_Context.hpp"
#include <algorithm>
#include <malloc.h>

namespace Vulkan_Memory {
    struct Last_Allocation {
        size_t size;
        size_t alignment;
        VkSystemAllocationScope allocScope;
    };

    struct MemoryHeader final : public Last_Allocation {};
    inline static Last_Allocation allocTracker;

    class Heap_Callback final {
        static Heap_Callback* m_singleton;
        static size_t m_heapCounter;
        static size_t m_allocCount;
        static constexpr size_t memory_header_size = sizeof(MemoryHeader);
        static constexpr size_t memory_header_alignment = alignof(MemoryHeader);
        public:
        Heap_Callback();
        ~Heap_Callback();

        #ifdef _GPLUSPLUS
        [[gnu::hot, gnu::alloc_size(2), gnu::alloc_align(3), gnu::malloc]]
        #endif
        inline static void* allocCallback(
            void* pUserdata,
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocScope
        ) noexcept
        #ifdef _CLANGPP
        __attribute__((hot, alloc_align(3), alloc_size(2), malloc))
        #endif
        {
            const size_t vkSize = size;
            size += memory_header_size;
            alignment = std::max(alignment, memory_header_alignment);

            size = (size + alignment - 1) & ~(alignment - 1);

            void* memory = std::aligned_alloc(alignment, size);
            
            Heap_Callback& memory_tracker = *m_singleton;

            memory_tracker.incriment();
            memory_tracker.add(size);

            allocTracker.size = size;
            allocTracker.alignment = alignment;
            allocTracker.allocScope = allocScope;

            MemoryHeader* memHeader = reinterpret_cast<MemoryHeader*>(memory);

            memHeader->size = vkSize;
            memHeader->alignment = alignment;
            memHeader->allocScope = allocScope;
            return reinterpret_cast<void*>((uint8_t*)memory + memory_header_size);
        }

        #ifdef _GPLUSPLUS
        [[gnu::hot, gnu::alloc_size(3), gnu::alloc_align(4), gnu::malloc]]
        #endif
        inline static void* reallocCallback(
            void* pUserdata,
            void* pOriginal,
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocScope
        ) noexcept
        #ifdef _CLANGPP
        __attribute__((hot, alloc_align(4), alloc_size(3), malloc))
        #endif
        {
            

            
        }

        private:
        inline void incriment() noexcept;
        inline void add(size_t size) noexcept;
        inline void sub(size_t size) noexcept;
    };
}