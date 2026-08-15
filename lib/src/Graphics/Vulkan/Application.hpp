#pragma once
#define VK_NO_PROTOTYPES

#include <volk.h>

#include <SDL3/SDL.h>

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <SDL3/SDL_vulkan.h>

#include <shaderc/shaderc.h>
#include <vk_mem_alloc.h>

#include <cstdint>
#include <vector>
#include <array>
#include <string_view>

namespace Djinn
{
    static constexpr uint32_t windowWidth { 800u };
    static constexpr uint32_t windowHeight { 1200u };
    static constexpr uint32_t vulkanVersion { VK_API_VERSION_1_4 };
    static constexpr uint32_t maxFramesInFlight { 2u };
    static constexpr VkFormat swapChainFormat { VK_FORMAT_B8G8R8A8_SRGB };
    static constexpr VkFormat depthFormat { VK_FORMAT_D32_SFLOAT };

    struct FrameResource final
    {
        VkCommandPool commandPool { VK_NULL_HANDLE };
        VkCommandBuffer commandBuffer { VK_NULL_HANDLE };
        VkSemaphore imageAcquiredSemaphore { VK_NULL_HANDLE };
    };

    class Application final
    {
        bool m_running { false };
        
        SDL_Window* m_window { nullptr };
        VkInstance m_instance { VK_NULL_HANDLE };
        VkPhysicalDevice m_physicalDevice { VK_NULL_HANDLE };
        VkDevice m_device { VK_NULL_HANDLE };
        VkSurfaceKHR m_surface { VK_NULL_HANDLE };
        VmaAllocator m_allocator { nullptr };

        uint64_t m_nextSignalValue { 3ULL };
        uint64_t m_frameCounter { 0ULL };
        uint64_t m_frameIdx { 0ULL };

        uint32_t m_gfxQueueFamIdx { UINT32_MAX };
        VkQueue m_gfxQueue { VK_NULL_HANDLE };

        VkSwapchainKHR m_swapChain { VK_NULL_HANDLE };

        bool m_requireSwapChainRecreate { false };

        uint32_t m_swapChainWidth { 0u };
        uint32_t m_swapChainHeight { 0u };

        VkImage m_depthImage { VK_NULL_HANDLE };
        VkImageView m_depthImageView { VK_NULL_HANDLE };
        VmaAllocation m_depthImageAllocation { VK_NULL_HANDLE };

        VkShaderModule m_vertShader { VK_NULL_HANDLE };
        VkShaderModule m_fragShader { VK_NULL_HANDLE };

        VkPipelineLayout m_pipelineLayout { VK_NULL_HANDLE };
        VkPipeline m_pipeline { VK_NULL_HANDLE };

        VkSemaphore m_timelineSemaphore { VK_NULL_HANDLE };
        std::array<FrameResource, maxFramesInFlight> m_frameResources;

        VkResult p_initInstance() noexcept;
        bool p_initPhysicalDevices() noexcept;

        void SDL_reportError( const char* errorMsg ) noexcept
        {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "Error",
                errorMsg,
                m_window
            );
        }

        bool initialiseVulkan() noexcept;



        public:
        bool initialiseApplication() noexcept;
    };
}