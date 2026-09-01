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
    inline static Last_Allocation allocTracker{};

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
        inline static VKAPI_ATTR void* allocCallback(
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

            if (
                memory == nullptr
            ) {
                return nullptr;
            }
            
            Heap_Callback& memory_tracker = *m_singleton;

            memory_tracker.incriment();
            memory_tracker.add(size);

            allocTracker.size = size;
            allocTracker.alignment = alignment;
            allocTracker.allocScope = allocScope;

            *reinterpret_cast<size_t*>((uint8_t*)memory + alignment - sizeof(size_t)) = alignment;

            MemoryHeader* memHeader = reinterpret_cast<MemoryHeader*>(memory);

            memHeader->size = vkSize;
            memHeader->alignment = alignment;
            memHeader->allocScope = allocScope;
            return reinterpret_cast<void*>((uint8_t*)memory + alignment);
        }

        #ifdef _GPLUSPLUS
        [[gnu::hot, gnu::alloc_size(3), gnu::alloc_align(4), gnu::malloc]]
        #endif
        inline static VKAPI_ATTR void* reallocCallback(
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
            if ( !pOriginal ) {
                return allocCallback(pUserdata, size, alignment, allocScope);
            } else {
                alignment = std::max(alignment, memory_header_alignment);
                uint8_t* memory = reinterpret_cast<uint8_t*>(pOriginal);
                MemoryHeader* memHeader = reinterpret_cast<MemoryHeader*>(memory - alignment * 3);
                
                void* newMemory = std::aligned_alloc(
                    memHeader->alignment,
                    memHeader->size
                );

                if (
                    newMemory == nullptr
                ) {
                    return nullptr;
                }

                MemoryHeader* newMemHeader = reinterpret_cast<MemoryHeader*>(newMemory);
                newMemHeader->size = memHeader->size;
                newMemHeader->alignment = memHeader->alignment;
                newMemHeader->allocScope = memHeader->allocScope;

                *reinterpret_cast<size_t*>((uint8_t*)newMemory + alignment - sizeof(size_t)) = alignment;
                std::free(pOriginal);

                return reinterpret_cast<void*>((uint8_t*)newMemory + alignment);
            }
        }

        inline static VKAPI_ATTR void freeCallback(
            void* pUserdata,
            void* pMemory
        ) noexcept {
            if ( !pMemory ) {
                return;
            }

            uint8_t* memory = reinterpret_cast<uint8_t*>(pMemory);
            size_t alignment = *reinterpret_cast<size_t*>(memory - sizeof(size_t));

            MemoryHeader* memHeader = reinterpret_cast<MemoryHeader*>(memory - alignment);
            Heap_Callback& memory_tracker = *m_singleton;

            memory_tracker.sub(memHeader->size);
            memory_tracker.decrement();

            std::free(pMemory);
        }

        private:
        inline void incriment() noexcept;
        inline void decrement() noexcept;
        inline void add(size_t size) noexcept;
        inline void sub(size_t size) noexcept;
    };
}