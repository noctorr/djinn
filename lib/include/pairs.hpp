#pragma once

#include <type_traits>

namespace utl
{
    template <auto f, auto y>
    struct holder
    {
        static constexpr decltype(f) first = f;
        static constexpr decltype(y) second = y;
    };

    template <auto f, auto y, auto z>
    struct _holder_t
    {
        static constexpr decltype(f) first = f;
        static constexpr decltype(y) second = y;
        static constexpr decltype(z) third = z;
    };
    /// @note unique, types
    template < 
        typename _f_Type,
        typename _s_Type,
        typename _t_Type,
        _f_Type firstVal,
        _s_Type secVal,
        _t_Type thirdVal
    >
    class u_ct_pair final
    {
        static constexpr _f_Type first_value = firstVal;
        static constexpr _s_Type second_value = secVal;
        static constexpr _t_Type third_value = thirdVal;

        static_assert(
            std::is_same_v<_f_Type, std::remove_cv<_f_Type>>,
            "Cannot bind a type's template parameter with a cv qualifier."
        );

        static_assert(
            std::is_same_v<_s_Type, std::remove_cv<_s_Type>>,
            "Cannot bind a type's template parameter with a cv qualifier."
        );

        static_assert(
            std::is_same_v<_t_Type, std::remove_cv<_t_Type>>,
            "Cannot bind a type's template parameter with a cv qualifier."
        );

        public:

        constexpr _holder_t<first_value, second_value, third_value> get ()
        {
            return { first_value, second_value, third_value };
        }
    };

    template <
        typename _f_Type,
        typename _s_Type,
        _f_Type firstVal,
        _s_Type secVal
    >
    class u_cs_pair final
    {
        static constexpr _f_Type first_value = firstVal;
        static constexpr _s_Type second_value = secVal;

        static_assert(
            std::is_same_v<_f_Type, std::remove_cv<_f_Type>>,
            "Cannot bind a type's template parameter with a cv qualifier."
        );

        static_assert(
            std::is_same_v<_s_Type, std::remove_cv<_s_Type>>,
            "Cannot bind a type's template parameter with a cv qualifier."
        );

        public:
        using firstType = _f_Type;
        using secondType = _s_Type;

        template < typename _Type  >
        requires ( (!std::is_rvalue_reference_v<_Type>) && std::is_same_v<_Type, _f_Type> )
        _f_Type* operator& ( _Type& x )
        {
            /// will do more on this
            return &firstVal;
        }

        constexpr holder<first_value, second_value> get ()
        {
            return { first_value, second_value };
        }
    };
}