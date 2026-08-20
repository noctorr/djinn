#pragma once
#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

namespace Djinn {
    static constexpr VkFormat swapChainFormat { VK_FORMAT_B8G8R8A8_SRGB };
    static constexpr uint32_t vulkanAPIVersion { VK_API_VERSION_1_4 };

    inline VkSurfaceKHR surface { VK_NULL_HANDLE };
    inline VkInstance* instPointer { nullptr };
    inline VkPhysicalDevice* physDevicePointer { nullptr };

    struct Instance final {
        VkInstance instance { VK_NULL_HANDLE };

        VkResult initInstance() noexcept;
    };

    struct PhysicalDevice final {
        VkPhysicalDevice device { VK_NULL_HANDLE };

        bool initPhysicalDevice() noexcept;
    }; 

    struct Swapchain final {
        VkSwapchainKHR swapchain { VK_NULL_HANDLE };
        uint32_t swapchainWidth { 0u };
        uint32_t swapchainHeight { 0u };

        bool initSwapchain(
            uint32_t,
            uint32_t
        ) noexcept;

        void destroySwapchain() noexcept;
        private:

    };


}