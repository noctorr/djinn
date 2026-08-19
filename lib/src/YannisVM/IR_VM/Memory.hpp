#pragma once

#include <cstdint>
#include <type_traits>
#include <initializer_list>
#include <tuple>
#include <optional>
#include <any>

/// @note Types must be a non-cost and non-volatile.
template < typename _IndexType, typename _ValueType >
requires ( std::is_same_v< typename std::remove_cv<_IndexType>::type, 
    _IndexType> &&
    std::is_same_v< typename std::remove_cv<_ValueType>::type,
    _ValueType> )
class [[nodiscard("The object, 'YannisHashMap' must be assigned to a variable.")]] YannisStackHashMap final
{
    public:
    explicit YannisStackHashMap() = default;
    private:
    #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc++23-extensions"
    _IndexType* m_indexVector { nullptr };
    _ValueType* m_valueVector { nullptr };
    std::size_t m_capacity, m_size, m_offset { 0uz };

    public:

    explicit YannisStackHashMap(
        std::initializer_list<
            _IndexType
        > _indexList,
        std::initializer_list<
            _ValueType
        > _valueList
    ) noexcept {
        const std::size_t indexSize = _indexList.size();
        const std::size_t valueSize = _valueList.size();
        const std::size_t trueSize = (indexSize > valueSize) ? indexSize : valueSize;
        m_size = trueSize;
        m_capacity = trueSize + 5uz;
        m_offset = trueSize - 1uz;

        m_indexVector = new _IndexType[m_capacity];
        m_valueVector = new _ValueType[m_capacity];

        std::size_t index { 0uz };
        for (
            typename std::initializer_list<_IndexType>::const_iterator value = _indexList.begin();
            value != _indexList.end();
            ++value, ++index)
        {
            m_indexVector[index] = *value;
        }

        index = 0uz;

        for (
            typename std::initializer_list<_ValueType>::const_iterator value = _valueList.begin();
            value != _valueList.end();
            ++value, ++index)
        {
            m_valueVector[index] = *value;
        }
    }

    explicit YannisStackHashMap(
        _IndexType index,
        _ValueType value
    ) noexcept {
        m_capacity = 5uz;
        m_size = 1uz;

        m_indexVector = new _IndexType[m_capacity];
        m_valueVector = new _ValueType[m_capacity];

        m_indexVector[0] = index;
        m_valueVector[0] = value;
    }

    YannisStackHashMap(
        const YannisStackHashMap&
    ) noexcept = delete;

    YannisStackHashMap(
        const YannisStackHashMap&&
    ) noexcept = delete;

    ~YannisStackHashMap() noexcept
    {
        delete[] m_indexVector;
        delete[] m_valueVector;
    }

    void operator= ( const YannisStackHashMap& ) noexcept = delete;
    void operator= ( const YannisStackHashMap&& ) noexcept = delete;

    void push (
        _IndexType _index,
        _ValueType _value
    ) noexcept {
        m_size++;
        if ( m_size > m_capacity )
        {
            m_capacity += 6uz;
            _IndexType cpy_Index[m_size];
            _ValueType cpy_Value[m_size];

            for ( std::size_t index { 0uz }; index < m_size; index++ )
            {
                cpy_Index[index] = m_indexVector[index];
                cpy_Value[index] = m_valueVector[index];
            }

            delete[] m_indexVector;
            delete[] m_valueVector;

            m_indexVector = new _IndexType[m_capacity];
            m_valueVector = new _ValueType[m_capacity];

            for ( std::size_t index { 0uz }; index < m_size; index++ )
            {
                m_indexVector[index] = cpy_Index[index];
                m_valueVector[index] = cpy_Value[index];
            }
        } else
        {
            m_offset++;
            m_indexVector[m_offset] = _index;
            m_valueVector[m_offset] = _value;
            if ( m_offset == m_size )
            {
                m_size++;
            }
        }
    }

    constexpr void pop ( ) noexcept
    {
        if constexpr ( std::is_arithmetic_v<_ValueType> && std::is_arithmetic_v<_IndexType> )
        {
            m_valueVector[m_offset] = static_cast<_ValueType>(0);
            m_indexVector[m_offset] = static_cast<_IndexType>(0);
            m_offset--;
        } else if constexpr ( std::is_arithmetic_v<_ValueType> && !std::is_arithmetic_v<_IndexType> )
        {
            m_valueVector[m_offset] = static_cast<_ValueType>(0);
            m_indexVector[m_offset] = nullptr;
            m_offset--;
        } else if constexpr ( std::is_arithmetic_v<_IndexType> )
        {
            m_valueVector[m_offset] = nullptr;
            m_indexVector[m_offset] = static_cast<_IndexType>(0);
            m_offset--;
        } else
        {
            m_valueVector[m_offset] = nullptr;
            m_indexVector[m_offset] = nullptr;
            m_offset--;
        }
    }

    std::tuple<const _ValueType&, const _IndexType&> top ( ) noexcept
    {
        return { m_valueVector[m_offset], m_indexVector[m_offset] };
    }

    std::optional<const _ValueType&> operator[] ( _IndexType _index ) noexcept
    {
        for ( std::size_t index { 0uz }; index < m_size; index++ )
        {
            if ( m_indexVector[index] == _index )
            {
                return m_valueVector[index];
            }
        }

        return std::nullopt;
    }

    #pragma clang diagnostic pop
    #else
    _IndexType* m_indexVector { nullptr };
    _ValueType* m_valueVector { nullptr };
    std::size_t m_capacity, m_size, m_offset { 0 };

    public:

    explicit YannisStackHashMap(
        std::initializer_list<
            _IndexType
        > _indexList,
        std::initializer_list<
            _ValueType
        > _valueList
    ) noexcept {
        const std::size_t indexSize = _indexList.size();
        const std::size_t valueSize = _valueList.size();
        const std::size_t trueSize = (indexSize > valueSize) ? indexSize : valueSize;
        m_size = trueSize;
        m_capacity = trueSize + 5;
        m_offset = trueSize - 1;

        m_indexVector = new _IndexType[m_capacity];
        m_valueVector = new _ValueType[m_capacity];

        std::size_t index { 0 };
        for (
            typename std::initializer_list<_IndexType>::const_iterator value = _indexList.begin();
            value != _indexList.end();
            ++value, ++index)
        {
            m_indexVector[index] = *value;
        }

        index = 0;

        for (
            typename std::initializer_list<_ValueType>::const_iterator value = _valueList.begin();
            value != _valueList.end();
            ++value, ++index)
        {
            m_valueVector[index] = *value;
        }
    }

    explicit YannisStackHashMap(
        _IndexType index,
        _ValueType value
    ) noexcept {
        m_capacity = 5;
        m_size = 1;

        m_indexVector = new _IndexType[m_capacity];
        m_valueVector = new _ValueType[m_capacity];

        m_indexVector[0] = index;
        m_valueVector[0] = value;
    }

    YannisStackHashMap(
        const YannisStackHashMap&
    ) noexcept = delete;

    YannisStackHashMap(
        const YannisStackHashMap&&
    ) noexcept = delete;

    ~YannisStackHashMap() noexcept
    {
        delete[] m_indexVector;
        delete[] m_valueVector;
    }

    void operator= ( const YannisStackHashMap& ) noexcept = delete;
    void operator= ( const YannisStackHashMap&& ) noexcept = delete;

    void push (
        _IndexType _index,
        _ValueType _value
    ) noexcept {
        m_size++;
        if ( m_size > m_capacity )
        {
            m_capacity += 6;
            _IndexType cpy_Index[m_size];
            _ValueType cpy_Value[m_size];

            for ( std::size_t index { 0 }; index < m_size; index++ )
            {
                cpy_Index[index] = m_indexVector[index];
                cpy_Value[index] = m_valueVector[index];
            }

            delete[] m_indexVector;
            delete[] m_valueVector;

            m_indexVector = new _IndexType[m_capacity];
            m_valueVector = new _ValueType[m_capacity];

            for ( std::size_t index { 0 }; index < m_size; index++ )
            {
                m_indexVector[index] = cpy_Index[index];
                m_valueVector[index] = cpy_Value[index];
            }
        } else
        {
            m_offset++;
            m_indexVector[m_offset] = _index;
            m_valueVector[m_offset] = _value;
            if ( m_offset == m_size )
            {
                m_size++;
            }
        }
    }

    constexpr void pop ( ) noexcept
    {
        if constexpr ( std::is_arithmetic_v<_ValueType> && std::is_arithmetic_v<_IndexType> )
        {
            m_valueVector[m_offset] = static_cast<_ValueType>(0);
            m_indexVector[m_offset] = static_cast<_IndexType>(0);
            m_offset--;
        } else if constexpr ( std::is_arithmetic_v<_ValueType> && !std::is_arithmetic_v<_IndexType> )
        {
            m_valueVector[m_offset] = static_cast<_ValueType>(0);
            m_indexVector[m_offset] = nullptr;
            m_offset--;
        } else if constexpr ( std::is_arithmetic_v<_IndexType> )
        {
            m_valueVector[m_offset] = nullptr;
            m_indexVector[m_offset] = static_cast<_IndexType>(0);
            m_offset--;
        } else
        {
            m_valueVector[m_offset] = nullptr;
            m_indexVector[m_offset] = nullptr;
            m_offset--;
        }
    }

    std::tuple<const _ValueType&, const _IndexType&> top ( ) noexcept
    {
        return { m_valueVector[m_offset], m_indexVector[m_offset] };
    }

    std::optional<const _ValueType&> operator[] ( _IndexType _index ) noexcept
    {
        for ( std::size_t index { 0 }; index < m_size; index++ )
        {
            if ( m_indexVector[index] == _index )
            {
                return m_valueVector[index];
            }
        }

        return std::nullopt;
    }
    #endif
};

class YannisStackPointer final
{
    YannisStackHashMap<uint16_t, std::any> m_memoryHashMap;
    std::size_t m_sizeOfStack;

    public:

    /// @note future logic here
};