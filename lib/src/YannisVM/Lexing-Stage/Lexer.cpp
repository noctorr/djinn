#include "Lexer.hpp"

#include <string>
#include <algorithm>
#include <unordered_map>

void vm::Lexer::tokenise () noexcept
{
    size_t start { 0uz };

    while (
        m_current < m_length
    )
    {
        char current = (*m_source)[m_current];

        if ( std::isspace(current) )
        {
            m_current++;
            continue;
        } else if ( std::isdigit(current) )
        {
            tokens.emplace_back(
                numberise()
            );
        } else if ( std::isalpha(current) )
        {
            tokens.emplace_back(
                identifier()
            );
        } else if ( current == '{' )
        {
            tokens.emplace_back(
                vectorise()
            );
        }
    }
}

std::unordered_map<
    char,
    vm::TokenType
> ttLiteralsHashMap = {
    { 'c', vm::TokenType::eight_bit },
    { 's', vm::TokenType::sixteen_bit },
    { 'i', vm::TokenType::thirty_two_bit },
    { 'l', vm::TokenType::sixty_four_bit },
    { 'f', vm::TokenType::single_precision },
    { 'd', vm::TokenType::double_precision },
    { 'b', vm::TokenType::triple_precision }
};

vm::Token vm::Lexer::numberise () noexcept
{
    std::string value;
    value.push_back((*m_source)[m_current]);
    m_current++;

    while (
        m_current < m_length
    )
    {
        if (
            std::isdigit(
                (*m_source)[m_current]
            )
        )
        {
            value.push_back((*m_source)[m_current]);
        } else
        {
            break;
        }
    }

    vm::TokenType token = vm::TokenType::double_precision; // default

    std::unordered_map<char, vm::TokenType>::const_iterator val = ttLiteralsHashMap.find((*m_source)[m_current]);

    if (
        val != ttLiteralsHashMap.end()
    )
    {
        token = val->second;
    }

    return vm::Token(token, value);
}