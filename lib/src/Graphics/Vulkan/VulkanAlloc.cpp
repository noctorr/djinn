#include "VulkanAlloc.hpp"
#include <cstdlib>
#include <cstring>

template < typename _Type >
void* Djinn::Alloc<_Type>::allocate( 
    std::size_t size,
    std::size_t alignment
) noexcept
#ifdef __cpp_concepts
requires ( std::is_pointer_v<_Type> )
#endif
{
    #if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSCV_LANG >= 201103L)
    static_assert(std::is_pointer_v<_Type>, "Type is not a pointer.");
    #endif

    #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc++23-extensions"
    if ( size == 0uz || alignment == 0uz )
    {
        return nullptr;
    }

    std::size_t alignedSize = (size + alignment - 1uz) / alignment * alignment;
    #pragma clang diagnostic pop
    #else
    if ( size == static_cast<std::size_t>(0) || alignment == static_cast<std::size_t>(0) )
    {
        return nullptr;
    }

    std::size_t alignedSize = (size + alignment - static_cast<std::size_t>(1)) / alignment * alignment;
    #endif

    return std::aligned_alloc(
        alignment,
        alignedSize
    );
}

template < typename _Type >
void* Djinn::Alloc<_Type>::reallocate(
    void* ptr,
    std::size_t size,
    std::size_t alignment
) noexcept 
#ifdef __cpp_concepts
requires ( std::is_pointer_v<_Type> )
#endif
{
    #if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSCV_LANG >= 201103L)
    static_assert(std::is_pointer_v<_Type>, "Type is not a pointer.");
    #endif

    #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc++23-extensions"

    if ( size == 0uz || alignment == 0uz )
    {
        return nullptr;
    }

    std::size_t alignedSize = (size + alignment - 1uz) / alignment * alignment;
    #pragma cland diagnostic pop
    #else
    if ( size == static_cast<std::size_t>(0) || alignment == static_cast<std::size_t>(0) )
    {
        return nullptr;
    }

    std::size_t alignedSize = (size + alignment - static_cast<std::size_t>(1)) / alignment * alignment;
    #endif

    _Type newPtr = static_cast<_Type>(
        std::aligned_alloc(
            alignedSize,
            size
        )
    );

    std::memcpy(
        newPtr,
        ptr,
        size
    );

    std::free(ptr);

    return newPtr;
}

template < typename _Type >
void Djinn::Alloc<_Type>::free(
    void* ptr
) noexcept {
    std::free(ptr);
}