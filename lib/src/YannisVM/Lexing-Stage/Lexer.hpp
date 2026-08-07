#pragma once

#include "Tokens.hpp"

namespace vm
{
    class Lexer final
    {
        std::string_view m_source;
        size_t m_counter { 0uz };
        

        public:
        explicit Lexer()
    };
}