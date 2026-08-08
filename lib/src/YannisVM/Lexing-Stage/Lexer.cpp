#include "Lexer.hpp"

#include <string>
#include <algorithm>
#include <unordered_map>

bool vm::Lexer::tokenise () noexcept
{
    m_running = true;
    while (
        m_current < m_length &&
        m_running
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
        } else if ( current == '[' )
        {
            tokens.emplace_back(
                vectorise()
            );
        } else if ( current == ';' )
        {
            tokens.emplace_back(
                vm::Token(vm::TokenType::semicolon, ";")
            );
        } else
        {
            tokens.emplace_back(
                operatorise()
            );
        }
        m_current++;
        continue;
    }

    tokens.emplace_back(vm::Token(vm::TokenType::eof, ""));
    return m_running;
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

std::unordered_map<
    std::string,
    vm::TokenType
> ttKeywordHashMap = {
    { "const", vm::TokenType::const_ },
    { "let", vm::TokenType::let },
    { "sizeof", vm::TokenType::sizeof_ },
    { "alignof", vm::TokenType::alignof_ },
    { "if", vm::TokenType::if_ },
    { "for", vm::TokenType::for_ },
    { "switch", vm::TokenType::switch_ },
    { "while", vm::TokenType::while_ },
    { "default", vm::TokenType::default_ },
    { "elif", vm::TokenType::elif },
    { "else", vm::TokenType::else_ },
    { "and", vm::TokenType::_and },
    { "or", vm::TokenType::_or },
    { "std", vm::TokenType::std_ },
    { "8i", vm::TokenType::eight_bit },
    { "16i", vm::TokenType::sixteen_bit },
    { "32i", vm::TokenType::thirty_two_bit },
    { "64i", vm::TokenType::sixty_four_bit },
    { "32f", vm::TokenType::single_precision },
    { "64f", vm::TokenType::double_precision },
    { "128f", vm::TokenType::triple_precision },
    { "fn", vm::TokenType::function }
};

vm::Token vm::Lexer::identifier ( ) noexcept
{
    std::string value;
    value.push_back((*m_source)[m_current]);
    m_current++;

    while (
        m_current < m_length
    )
    {
        if (
            std::isalpha(
                (*m_source)[m_current]
            ) ||
            std::isdigit(
                (*m_source)[m_current]
            )
        )
        {
            value.push_back(
                (*m_source)[m_current]
            );
            m_current++;
            continue;
        } else
        {
            break;
        }
    }

    std::unordered_map<std::string, vm::TokenType>::const_iterator val = ttKeywordHashMap.find(value);

    if (
        val != ttKeywordHashMap.end()
    )
    {
        return vm::Token(val->second, value);
    } else
    {
        return vm::Token(vm::TokenType::varname, value);
    }
}

std::unordered_map<
    std::string,
    vm::TokenType
> ttOperatorHashMap = {
    { "=>", vm::TokenType::left_greater_or_equal },
    { "=<", vm::TokenType::right_greater_or_equal },
    { "==", vm::TokenType::is_equal },
    { "!=", vm::TokenType::does_not_equal },
    { "=", vm::TokenType::equal_sign },
    { "!", vm::TokenType::not_bool },
    { "<", vm::TokenType::right_greater },
    { ">", vm::TokenType::left_greater }
};

vm::Token vm::Lexer::operatorise ( ) noexcept
{
    std::string value;
    value.push_back((*m_source)[m_current]);

    std::unordered_map<std::string, vm::TokenType>::const_iterator val = ttOperatorHashMap.find(value);

    #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc++23-extensions"
    if (
        val != ttOperatorHashMap.end()
    )
    {
        if ( val->first == "!" &&
            (*m_source)[m_current+1uz] == '='
        )
        {
            value.push_back('=');
            m_current += 2uz;
            return vm::Token(vm::TokenType::does_not_equal, value);
        } else if ( val->first == "=" &&
            (*m_source)[m_current+1uz] == '='
        )
        {
            value.push_back('=');
            m_current += 2uz;
            return vm::Token(vm::TokenType::is_equal, value);
        } else
        {
            return vm::Token(val->second, value);
        }
    } else
    {
        value.push_back(
            (*m_source)[m_current+1uz]
        );

        std::unordered_map<std::string, vm::TokenType>::const_iterator val_2 = ttOperatorHashMap.find(value);

        if ( val_2 != ttOperatorHashMap.end() )
        {
            return vm::Token(val_2->second, value);
        } else
        {
            m_running = false;
            return vm::Token(vm::TokenType::error, "");
        }
    }
    #pragma clang diagnostic pop
    #endif 
}