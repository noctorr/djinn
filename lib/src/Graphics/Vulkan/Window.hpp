#include <SDL3/SDL.h>

namespace Djinn {
    inline static SDL_Window* window          { nullptr };

    inline static constexpr uint32_t width    {  1200u  };
    inline static constexpr uint32_t height   {  800u   };

    inline void SDL_ReportError( const char* pMessage ) noexcept
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Error",
            pMessage,
            window
        );
    }

    [[nodiscard]] inline bool createWindow() noexcept
    {
        if ( !SDL_InitSubSystem(SDL_INIT_VIDEO) )
        {
            SDL_ReportError("Could not initialise SDL.");
            return false;
        }

        window = SDL_CreateWindow(
            "Djinn - Game Engine",
            width, height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
        );

        if ( !window )
        {
            SDL_ReportError(SDL_GetError());
            return false;
        }

        return true;
    }
}