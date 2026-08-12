#pragma once
#include <cstdint>
#include <type_traits>

namespace Djinn
{
    template < typename _Type >
    struct Alloc final
    {
        void* allocate(
            std::size_t size,
            std::size_t alignment
        ) noexcept
        #ifdef __cpp_concepts
        requires ( std::is_pointer_v<_Type> )
        #endif
        ;

        void* reallocate(
            void* ptr,
            std::size_t size,
            std::size_t alignment
        ) noexcept
        #ifdef __cpp_concepts
        requires ( std::is_pointer_v<_Type> )
        #endif
        ;

        void free(
            void* ptr
        ) noexcept;
    };
}