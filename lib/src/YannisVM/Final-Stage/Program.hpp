#pragma once

#include "../Parsing-Stage/Parser.hpp"

namespace vm {
    struct Program final
    {
        std::string_view text;

        explicit Program(
            std::string_view _text
        ) : text(std::move(_text))
        {}

        void initialise() noexcept;
    };
}