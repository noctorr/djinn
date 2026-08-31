#include "Vulkan_Alloc.hpp"

namespace Vulkan_Memory {
    Heap_Callback* Heap_Callback::m_singleton { nullptr };
    size_t Heap_Callback::m_heapCounter { 0uz };
    size_t Heap_Callback::m_allocCount { 0uz };

    Heap_Callback::Heap_Callback()
    {
        if (!m_singleton) {
            m_singleton = this;
        }
    }

    Heap_Callback::~Heap_Callback()
    {
        if ( m_singleton == this )
        {
            m_singleton = nullptr;
        }
    }

    inline void Heap_Callback::incriment() noexcept
    {
        m_allocCount++;
    }

    inline void Heap_Callback::decrement() noexcept
    {
        m_allocCount--;
    }

    inline void Heap_Callback::add(size_t size) noexcept
    {
        m_heapCounter += size;
    }

    inline void Heap_Callback::sub(size_t size) noexcept
    {
        m_heapCounter -= size;
    }
}