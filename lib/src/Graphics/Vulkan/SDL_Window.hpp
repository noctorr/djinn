#pragma once

#include <SDL3/SDL.h>

namespace Djinn_SDL {
    struct Window final
    {
        static constexpr uint32_t width { 1200u };
        static constexpr uint32_t height { 800u };

        SDL_Window* window   { nullptr };
        
        [[nodiscard]] bool init() noexcept;
        void shutdown() noexcept;
        void ReportError(const char* pMessage) noexcept;
    };
}