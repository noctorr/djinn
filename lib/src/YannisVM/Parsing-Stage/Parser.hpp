#pragma once

#include "AST.hpp"

namespace vm {
    enum class VmResult : unsigned char
    {
        SUCCESS,
        SYNTAX,
        OVERFLOW,
        SAFETY,
        ARITHMETIC,
        UNKOWN
    };

    enum class ERROR_MSG : unsigned char
    {
        
    };

    class Parser final
    {
        std::unique_ptr<std::vector<Token>> m_tokens;
        public:
        size_t current{};
        bool code { false };
        explicit Parser(
            std::string_view m_source
        ) noexcept {
            Lexer lexer { std::make_unique<std::string_view>(m_source) };
            bool result = lexer.tokenise();

            if ( result )
            {
                m_tokens = std::move(lexer.tokens);
            } else
            {
                return;
            }
        }

        [[nodiscard]] VmResult Parse() noexcept;
        private:
        

    };
}