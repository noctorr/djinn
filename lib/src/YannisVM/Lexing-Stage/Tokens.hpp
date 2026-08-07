#pragma once

#include <string_view>
#include <memory>

namespace vm
{
    enum class TokenType : unsigned char
    {
        // arithmetic operators
        add,
        mul,
        div,
        sub,

        // other operators
        open_param, // (
        close_param, // )
        open_braces, // {
        close_braces, // }
        open_square, // [
        close_square, // ]

        // logical operations
        equal_sign, // =
        not_bool, // !
        is_bool, // eg. if ( true )
        does_not_equal, // !=
        is_equal, // ==
        left_greater, // >
        right_greater, // <
        left_greater_or_equal, // =>
        right_greater_or_equal, // =<

        // types

        /// @cond Floats
        single_precision, // 32-bit float
        double_precision, // 64-bit float
        triple_precision, // 128-bit float
        /// @endcond

        /// @cond Integers
        boolean, // still has to be stored as a byte, may include bit-packing in boolean arrays
        eight_bit,
        sixteen_bit,
        thirty_two_bit,
        sixty_four_bit,
        /// @endcond

        /// @cond Strings
        character,
        string,
        /// @endcond

        // statement declare
        if_,
        for_,
        while_,
        switch_,
        case_,
        default_,

        // keywords
        sizeof_,
        alignof_,

        // grammar
        const_,

        // file data
        semicolon,
        eof
    };

    struct Token final
    {
        TokenType tokenType;
        std::unique_ptr<std::string_view> value;

        explicit Token(
            TokenType _tt,
            std::string_view _val
        ) : tokenType(_tt), value(std::move(std::make_unique<std::string_view>(_val)))
        {}
    };
}