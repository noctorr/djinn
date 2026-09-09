#pragma once

#include "Vulkan_Signals.hpp"
#include "Vulkan_Alloc.hpp"
#include "SDL_Window.hpp"

#include <vector>
#include <array>
#include <inplace_vector>
#include <tuple>

namespace Djinn_Vulkan {
    inline static Vulkan_Memory::Heap_Callback standalone{};

    enum class ShaderType : unsigned char {
        Vertex,
        Fragment,
        Geomery,
        Tesselation,
        Compute
    };

    class PipelineManager final
    {
        VkPipeline                 m_gfxPipeline        { VK_NULL_HANDLE };
        VkPipelineLayout           m_gfxLayout          { VK_NULL_HANDLE };

        public:
        [[nodiscard]] bool init_Pipeline(
            std::inplace_vector<std::tuple<VkShaderModule*, ShaderType>, 5>&
        ) noexcept;
    };

    class SwapchainManager final
    {
        static constexpr VkFormat desiredFormat              =   VK_FORMAT_B8G8R8A8_SRGB;
        static constexpr VkColorSpaceKHR desiredColorSpace   =   VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        static constexpr VkPresentModeKHR desiredPresentMode =   VK_PRESENT_MODE_MAILBOX_KHR;

        static constexpr VkFormat depthFormat                =   VK_FORMAT_D32_SFLOAT;

        std::vector<VkImage>       m_images;
        std::vector<VkImageView>   m_imageViews;
        VkSwapchainKHR             m_swapChain               { VK_NULL_HANDLE };
        SDL_Window*                m_window                  { nullptr };

        VkImage                    m_depthImage              { VK_NULL_HANDLE };
        VkImageView                m_depthImageView          { VK_NULL_HANDLE };
        VmaAllocation              m_depthImageAllocation    { VK_NULL_HANDLE };

        VkSurfaceFormat2KHR        m_surfaceFormat;
        VkPresentModeKHR           m_presentMode;
        public:
        std::vector<VkSemaphore>   renderCompleteSemaphores;
        explicit SwapchainManager(
            SDL_Window* window
        ) : m_window(window) {}

        [[nodiscard]] bool init_Swapchain(
            VkDevice&,
            VkPhysicalDevice&,
            VkSurfaceKHR&,
            const VkAllocationCallbacks*,
            VmaAllocator&
        ) noexcept;

        void shutdown(
            VkDevice&,
            const VkAllocationCallbacks*,
            VmaAllocator&
        ) noexcept;
        private:
        VkExtent2D getSwapExtent(
            VkSurfaceCapabilities2KHR const&
        ) noexcept;
    };

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
        uint32_t                   userAPIVersion;
        bool                       hasDesiredFeatures   { false };

        std::array<VkBool32, 3>    featuresData;

        [[nodiscard]] VkResult init_instance()    noexcept override;
        [[nodiscard]] bool init_physicalDevice()  noexcept override;
        [[nodiscard]] bool init_gfxQueue()        noexcept override;
        [[nodiscard]] bool init_logicalDevice()   noexcept override;
        private:
        const VkAllocationCallbacks m_allocCallback
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

        const VkAllocationCallbacks* getAllocCallback() noexcept {
            return &m_allocCallback;
        }
    };
}