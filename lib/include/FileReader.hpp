#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
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

        file.close();

        return content;
    }

    struct FileMapContent final
    {
        std::string content;
        bool callback = false;

        FileMapContent(
            const std::filesystem::path&& filePath
        ) noexcept {
            std::ifstream file(filePath);
            
            if ( !file )
            {
                return;
            } else
            {
                callback = true;
                std::string fileContent {
                    std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>()
                };
                content = std::move(fileContent);
                file.close();
            }
        }

        FileMapContent(
            const std::filesystem::path& filePath
        ) noexcept {

        }
    };
}