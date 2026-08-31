#pragma once

#include "Vulkan_Context.hpp"

namespace Djinn_Vulkan {
    class VulkanSignal
    {
        static VulkanSignal* m_singleton; 
        public:
        VulkanSignal();
        virtual ~VulkanSignal();
        [[nodiscard]] virtual VkResult init_instance() noexcept = 0;
        [[nodiscard]] virtual bool init_physicalDevice() noexcept = 0;
        [[nodiscard]] virtual bool init_gfxQueue() noexcept = 0;
        [[nodiscard]] virtual bool init_logicalDevice() noexcept = 0;

        static VulkanSignal* get_singleton()
        {
            return m_singleton;
        }
    };
}