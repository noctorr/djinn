#include "SDL_Window.hpp"

namespace Djinn_SDL {
    [[nodiscard]] bool Window::init() noexcept
    {
        if (
            SDL_InitSubSystem(SDL_INIT_VIDEO)
        ) {
            window = SDL_CreateWindow(
                "Djinn - Game Engine",
                width, height,
                SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
            );

            if (
                window
            ) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        } 
    }

    void Window::shutdown() noexcept {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Window::ReportError(const char* pMessage) noexcept {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Error Code: 404",
            pMessage,
            window
        );
    }
}