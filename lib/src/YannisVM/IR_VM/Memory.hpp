#pragma once

#include <cstdint>
#include <type_traits>
#include <initializer_list>

/// @note Types must be a non-cost and non-volatile.
template < typename _IndexType, typename _ValueType >
requires ( std::is_same_v< typename std::remove_cv<_IndexType>::type, 
    _IndexType> &&
    std::is_same_v< typename std::remove_cv<_ValueType>::type,
    _ValueType> )
class [[nodiscard("The object, 'YannisHashMap' must be assigned to a variable.")]] YannisHashMap final
{
    _IndexType* m_indexVector { nullptr };
    _ValueType* m_valueVector { nullptr };
    public:
    explicit YannisHashMap(
        std::initializer_list<
            _IndexType
        > _indexList,
        std::initializer_list<
            _ValueType
        > _valueList
    ) {
        m_indexVector = new _IndexType[_indexList.size()];
        m_valueVector = new _ValueType[_valueList.size()];

        /// i looked at the initializer_list source code and a const_iterator is just a 'const type *' makes sense since its suppose to do its job at compile-time
        /// was thinking it was a struct

        #if defined(__clang__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wc++23-extensions"
        std::size_t index { 0uz };
        for (
            typename std::initializer_list<_IndexType>::const_iterator value = _indexList.begin();
            value != _indexList.end();
            ++value, ++index)
        {
            m_indexVector[index] = *value;
        }

        index = 0uz;
        #pragma clang diagnostic pop
        #endif

        for (
            typename std::initializer_list<_ValueType>::const_iterator value = _valueList.begin();
            value != _valueList.end();
            ++value, ++index)
        {
            m_valueVector[index] = *value;
        }
    }

    YannisHashMap(
        const YannisHashMap&
    ) = delete;

    YannisHashMap(
        YannisHashMap&&
    ) = delete;

    ~YannisHashMap()
    {
        delete[] m_indexVector;
        delete[] m_valueVector;
    }

    void operator= ( const YannisHashMap& ) = delete;
    void operator= ( YannisHashMap&& ) = delete;


};

class YannisStack final
{
    
    public:
};