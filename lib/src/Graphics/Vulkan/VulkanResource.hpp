#pragma once
#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#include <shaderc/shaderc.hpp>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <vector>
#include <array>

#include "Window.hpp"

namespace Djinn {
    inline static constexpr VkFormat swapChainFormat   { VK_FORMAT_B8G8R8A8_SRGB };
    inline static constexpr uint32_t vulkanAPIVersion  {   VK_API_VERSION_1_4    };
    inline static constexpr VkFormat depthFormat       { VK_FORMAT_D32_SFLOAT    };

    inline static VkSurfaceKHR surface                 { VK_NULL_HANDLE };
    inline static VkQueue gfxQueue                     { VK_NULL_HANDLE };

    inline static VkInstance* pInstance                { nullptr };
    inline static VkPhysicalDevice* pPhysicalDevice    { nullptr };
    inline static VkDevice* pDevice                    { nullptr };
    inline static VkSemaphore* pSemaphore              { nullptr };

    inline static uint32_t gfxQueueFamIdx              { UINT32_MAX };

    inline static VmaAllocator vkAllocator             { VK_NULL_HANDLE };
    inline static VmaAllocation depthImgAllocation     { VK_NULL_HANDLE };

    [[nodiscard]] inline static bool initVMA() noexcept;

    inline static VkPipelineLayout pipelineLayout      { VK_NULL_HANDLE };

    inline static VkShaderModule vertShader            { VK_NULL_HANDLE };
    inline static VkShaderModule fragShader            { VK_NULL_HANDLE };

    inline static std::vector<VkSemaphore> renCmpSem;
    inline static std::vector<VkImage> swapchainImages;
    inline static std::vector<VkImageView> swapchainImageViews;

    inline static VkImageView depthImageView           { VK_NULL_HANDLE };
    inline static VkImage depthImage                   { VK_NULL_HANDLE };

    [[nodiscard]] static inline bool initShaders() noexcept;

    struct Instance final {
        VkInstance instance      { VK_NULL_HANDLE };

        VkResult initInstance() noexcept;
    };

    struct PhysicalDevice final {
        VkPhysicalDevice device   { VK_NULL_HANDLE };

        [[nodiscard]] bool initPhysicalDevice() noexcept;
    };

    struct Device final {
        VkDevice device           { VK_NULL_HANDLE };

        [[nodiscard]] bool initDevice() noexcept;
        [[nodiscard]] bool initSynchronisation() noexcept;
        [[nodiscard]] bool initCommandBuffers() noexcept;
    };

    struct Swapchain final {
        VkSwapchainKHR swapchain   { VK_NULL_HANDLE };

        uint32_t swapchainWidth    { 0u };
        uint32_t swapchainHeight   { 0u };

        [[nodiscard]] bool initSwapchain(
            uint32_t,
            uint32_t
        ) noexcept;

        void destroySwapchain() noexcept;
    };

    struct Pipeline final {
        VkPipeline pipeline        { VK_NULL_HANDLE };

        [[nodiscard]] bool initGFXQueue() noexcept;
        [[nodiscard]] bool initGFXPipeline() noexcept;
    };

    struct Frame final
    {
        VkCommandPool commandPool           { VK_NULL_HANDLE };
        VkCommandBuffer commandBuffer       { VK_NULL_HANDLE };
        VkSemaphore imageAccquiredSemaphore { VK_NULL_HANDLE };
    };

    inline static std::array<Frame, 2> frameResources;
}