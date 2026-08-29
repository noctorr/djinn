#pragma once

#include "Vulkan_Signals.hpp"
#include "Vulkan_Alloc.hpp"
#include "SDL_Window.hpp"

namespace Djinn_Vulkan {
    class LogicalDeviceDriver final : public VulkanSignal
    {
        VkPhysicalDevice           m_physicalDevice     { VK_NULL_HANDLE };
        VkInstance                 m_instance           { VK_NULL_HANDLE };
        VkSurfaceKHR               m_surface            { VK_NULL_HANDLE };
        VkQueue                    m_gfxQueue           { VK_NULL_HANDLE };
        VkDevice                   m_logicalDevice      { VK_NULL_HANDLE };

        Djinn_SDL::Window            m_window;
        static uint32_t              UserAPIVersion;
        public:
        [[nodiscard]] VkResult init_instance() noexcept override;
        [[nodiscard]] bool init_physicalDevice() noexcept override;
        [[nodiscard]] bool init_gfxQueue() noexcept override;
        [[nodiscard]] bool init_logicalDevice() noexcept override;
        private:
        [[nodiscard]] uint8_t scoreDeviceCapabilities(
            const VkPhysicalDeviceFeatures& deviceFeatures,
            const VkPhysicalDeviceProperties& deviceProperties,
            const VkPhysicalDeviceMemoryProperties& deviceMemProperties 
        ) noexcept;

        [[nodiscard]] bool createSurface() noexcept;
    };
}