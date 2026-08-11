#pragma once

#include "../Lexing-Stage/Lexer.hpp"
#include <type_traits>


template < typename _Type >
struct Expression
{
    using type = _Type;
    virtual ~Expression();
};

template < typename _Type >
struct ASTNode final
{
    ASTNode* left;
    ASTNode* right;
    Expression<_Type> expr; 

    ASTNode(
        ASTNode* _left,
        ASTNode* _right
    ) : left(_left), right(_right)
    {}
};

// Counts as HexaDecimal
struct BinaryExpression : public Expression<long long>
{
    std::unique_ptr<Expression> expr;
    long long value;
    BinaryExpression(
        std::unique_ptr<Expression> _expr,
        long long _val
    ) : expr(std::move(_expr)), value(_val)
    {}
};

struct StringExpression : public Expression<const char*>
{
    std::unique_ptr<Expression> expr;
    const char* value;
    StringExpression(
        std::unique_ptr<Expression> _expr,
        const char* _val
    ) : expr(std::move(_expr)), value(_val)
    {}
};

template < typename _FloatType >
requires ( std::is_floating_point_v<_FloatType> )
struct FloatExpression : public Expression<_FloatType>
{
    std::unique_ptr<Expression<_FloatType>> expr;
    _FloatType value;
    FloatExpression(
        std::unique_ptr<Expression<_FloatType>> _expr,
        _FloatType _val
    ) : expr(std::move(_expr)), value(_val)
    {}
};

struct CharacterExpression : public Expression<char>
{
    std::unique_ptr<Expression> expr;
    char value;
    CharacterExpression(
        std::unique_ptr<Expression> _expr,
        char _val
    ) : expr(std::move(_expr)), value(_val)
    {}
};

template < typename _IntegerType >
requires ( std::is_integral_v<_IntegerType> )
struct IntegerExpression : public Expression<_IntegerType>
{
    std::unique_ptr<Expression<_IntegerType>> expr;
    _IntegerType value;
    IntegerExpression(
        std::unique_ptr<Expression<_IntegerType>> _expr,
        _IntegerType _val
    ) : expr(std::move(_expr)), value(_val)
    {}

    _IntegerType Add ( ) noexcept
    {}
    _IntegerType Sub ( ) noexcept
    {}
    _IntegerType Mul ( ) noexcept
    {}
    _IntegerType Div ( ) noexcept
    {}
};

struct VariableExpression : public Expression<const char*>
{
    std::unique_ptr<Expression> expr;
    const char* value;
    VariableExpression(
        std::unique_ptr<Expression> _expr,
        const char* _val
    ) : expr(std::move(_expr)), value(_val)
    {}
};