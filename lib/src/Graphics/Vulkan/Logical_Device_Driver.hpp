#pragma once

#include "Vulkan_Signals.hpp"
#include "Vulkan_Alloc.hpp"
#include "SDL_Window.hpp"

namespace Djinn_Vulkan {
    inline static Vulkan_Memory::Heap_Callback standalone{};
    class LogicalDeviceDriver final : public VulkanSignal
    {
        VkPhysicalDevice           m_physicalDevice     { VK_NULL_HANDLE };
        VkInstance                 m_instance           { VK_NULL_HANDLE };
        VkSurfaceKHR               m_surface            { VK_NULL_HANDLE };
        VkQueue                    m_gfxQueue           { VK_NULL_HANDLE };
        VkDevice                   m_logicalDevice      { VK_NULL_HANDLE };

        Djinn_SDL::Window          m_window;
        uint32_t                   m_gfxFamIdx;
        public:
        uint32_t                   m_userAPIVersion;

        [[nodiscard]] VkResult init_instance()    noexcept override;
        [[nodiscard]] bool init_physicalDevice()  noexcept override;
        [[nodiscard]] bool init_gfxQueue()        noexcept override;
        [[nodiscard]] bool init_logicalDevice()   noexcept override;
        private:
        VkAllocationCallbacks m_allocCallback
        {
            .pUserData = nullptr,
            .pfnAllocation = (PFN_vkAllocationFunction)Vulkan_Memory::Heap_Callback::allocCallback,
            .pfnReallocation = (PFN_vkReallocationFunction)Vulkan_Memory::Heap_Callback::reallocCallback,
            .pfnFree = (PFN_vkFreeFunction)Vulkan_Memory::Heap_Callback::freeCallback,
            .pfnInternalAllocation = [](
                void* pUserdata,
                size_t size,
                VkInternalAllocationType allocType,
                VkSystemAllocationScope allocScope
            ) -> void VK_LAMBDA_CALL {},
            .pfnInternalFree = [](
                void* pUserdata,
                size_t size,
                VkInternalAllocationType allocType,
                VkSystemAllocationScope allocScope
            ) -> void VK_LAMBDA_CALL {}
        };
        [[nodiscard]] uint8_t scoreDeviceCapabilities(
            const VkPhysicalDeviceFeatures& deviceFeatures,
            const VkPhysicalDeviceProperties& deviceProperties,
            const VkPhysicalDeviceMemoryProperties& deviceMemProperties 
        ) noexcept;

        public:
        VkPhysicalDevice& getPhysicalDevice() noexcept {
            return m_physicalDevice;
        }

        VkInstance& getInstance() noexcept {
            return m_instance;
        }

        VkSurfaceKHR& getSurface() noexcept {
            return m_surface;
        }

        VkQueue& getGFXQueue() noexcept {
            return m_gfxQueue;
        }

        VkDevice& getLogicalDevice() noexcept {
            return m_logicalDevice;
        }

        VkAllocationCallbacks* getAllocCallback() noexcept {
            return &m_allocCallback;
        }
    };
}