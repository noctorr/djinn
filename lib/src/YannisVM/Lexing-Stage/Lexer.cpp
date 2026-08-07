#include "Lexer.hpp"

#include <string>
#include <algorithm>

void vm::Lexer::tokenise () noexcept
{
    size_t start { 0uz };

    while (
        m_current < (*m_source).length() 
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

