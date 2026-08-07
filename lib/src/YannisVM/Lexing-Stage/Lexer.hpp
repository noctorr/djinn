#pragma once

#include "Tokens.hpp"
#include <vector>

namespace vm
{
    class Lexer final
    {
        std::unique_ptr<std::string_view> m_source;
        #if defined(__clang__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wc++23-extensions"
        size_t m_current { 0uz };
        #pragma clang diagnostic pop
        #endif
        public:
        explicit Lexer(
            std::unique_ptr<std::string_view> _source
        ) noexcept : m_source(std::move(_source))
        {}

        std::vector<Token> tokens;
        void tokenise() noexcept;

        private:
        Token numberise() noexcept;
        Token vectorise() noexcept;
        Token identifier() noexcept;
    };
}