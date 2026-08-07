#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <optional>

namespace ftl
{
    [[nodiscard]] inline std::optional<std::string> readFile ( const std::filesystem::path& filePath ) noexcept
    {
        std::ifstream file(filePath);

        if (!file)
        {
            return std::nullopt;
        }

        std::string content {
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };

        return content;
    }

    struct FileMapContent final
    {
        
    };
}