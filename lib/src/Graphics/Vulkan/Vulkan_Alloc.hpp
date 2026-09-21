#pragma once

#include "Vulkan_Context.hpp"
#include <algorithm>
#include <new>
#include <cstring>

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
        inline static VKAPI_ATTR void* VKAPI_CALL allocCallback(
            void* pUserdata,
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocScope
        ) noexcept
        #ifdef _CLANGPP
        __attribute__((hot, alloc_align(3), alloc_size(2), malloc))
        #endif
        {
            alignment = std::max(alignment, memory_header_alignment);
            const size_t totalSize = memory_header_size + sizeof(void*) + size + alignment;
            void* memory = operator new(totalSize, std::align_val_t{alignment}, std::nothrow);

            if (
                !memory
            ) {
                return nullptr;
            }
            
            Heap_Callback& memory_tracker = *m_singleton;

            memory_tracker.incriment();
            memory_tracker.add(size);

            std::byte* base = static_cast<std::byte*>(memory);
            std::byte* payload = base + memory_header_size + sizeof(void*);
            payload = reinterpret_cast<std::byte*>(
                (reinterpret_cast<uintptr_t>(payload) + alignment - 1) & ~(alignment - 1)
            );

            *reinterpret_cast<void**>(payload - sizeof(void*)) = memory;

            allocTracker.size = size;
            allocTracker.alignment = alignment;
            allocTracker.allocScope = allocScope;

            MemoryHeader* memHeader = reinterpret_cast<MemoryHeader*>(base);

            memHeader->size = size;
            memHeader->alignment = alignment;
            memHeader->allocScope = allocScope;
            return reinterpret_cast<void*>(payload);
        }

        #ifdef _GPLUSPLUS
        [[gnu::hot, gnu::alloc_size(3), gnu::alloc_align(4), gnu::malloc]]
        #endif
        inline static VKAPI_ATTR void* VKAPI_CALL reallocCallback(
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
            if (!pOriginal) {
                return allocCallback(
                    pUserdata,
                    size,
                    alignment,
                    allocScope
                );
            } else {
                void* base = *reinterpret_cast<void**>(
                    reinterpret_cast<std::byte*>(
                        pOriginal
                    ) - sizeof(void*) 
                );

                MemoryHeader* header = reinterpret_cast<MemoryHeader*>(base);
                const size_t oldSize = header->size;

                alignment = std::max(
                    alignment,
                    header->alignment
                );
                const size_t copySize = std::min(oldSize, size);
                const size_t totalSize = memory_header_size + sizeof(void*) + size + alignment;

                void* memory = operator new(
                    totalSize,
                    std::align_val_t{alignment},
                    std::nothrow
                );

                if (!memory) {
                    return nullptr;
                }

                std::byte* newBase = static_cast<std::byte*>(memory);
                std::byte* newPayload = newBase + memory_header_size + sizeof(void*);
                newPayload = reinterpret_cast<std::byte*>(
                    (reinterpret_cast<uintptr_t>(
                        newPayload
                    ) + alignment - 1) & ~(alignment - 1)
                );

                *reinterpret_cast<void**>(newPayload - sizeof(void*)) = memory;

                MemoryHeader* memHeader = reinterpret_cast<MemoryHeader*>(newBase);
                memHeader->size = size;
                memHeader->alignment = alignment;
                memHeader->allocScope = allocScope;

                std::memcpy(newPayload, pOriginal, copySize);

                Heap_Callback& memory_tracker = *m_singleton;

                memory_tracker.sub(oldSize);
                memory_tracker.add(size);

                allocTracker.size = size;
                allocTracker.alignment = alignment;
                allocTracker.allocScope = allocScope;

                operator delete(base, std::align_val_t{header->alignment});
                return reinterpret_cast<void*>(newPayload);
            }
        }

        inline static VKAPI_ATTR void VKAPI_CALL freeCallback(
            void* pUserdata,
            void* pMemory
        ) noexcept {
            if (
                !pMemory
            ) {
                return;
            }

            void* base = *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pMemory) - sizeof(void*));
            MemoryHeader* memHeader = reinterpret_cast<MemoryHeader*>(base);

            Heap_Callback& memory_tracker = *m_singleton;
            memory_tracker.decrement();
            memory_tracker.sub(memHeader->size);

            operator delete(base, std::align_val_t{memHeader->alignment});
        }

        private:
        inline void incriment() noexcept;
        inline void decrement() noexcept;
        inline void add(size_t size) noexcept;
        inline void sub(size_t size) noexcept;
    };
}