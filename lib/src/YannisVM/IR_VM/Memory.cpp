#include "Memory.hpp"

YannisStackPointer::YannisStackPointer(
    std::initializer_list<uint16_t> ids,
    std::initializer_list<std::any> value
) noexcept {
    m_sizeOfStack += m_memoryHashMap(ids, value);

    if ( m_sizeOfStack >= STACKMAXSIZE )
    {
        /// @note will do something
    }
}