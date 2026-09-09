#include "Logical_Device_Driver.hpp"
#include <SDL3/SDL_vulkan.h>

#include "Vulkan_Debug.hpp"

#include <vector>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <limits>

#define MAX_HEAP_SIZE 8.0

namespace Djinn_Vulkan {
    static constexpr double MEM_CONSTANT = 1024.0 * 1024.0 * 1024.0;

    [[nodiscard]] uint8_t LogicalDeviceDriver::scoreDeviceCapabilities(
        const VkPhysicalDeviceFeatures& deviceFeatures,
        const VkPhysicalDeviceProperties& deviceProperties,
        const VkPhysicalDeviceMemoryProperties& deviceMemProperties 
    ) noexcept {
        uint8_t Score{};
        if (
            deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        ) {
            Score += 5;
        } else if (
            deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
        ) {
            Score += 3;
        } else if (
            deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU
        ) {
            Score++;
        }

        double highestGB { 0.0 };
        for (
            const VkMemoryHeap& heap : deviceMemProperties.memoryHeaps
        ) {
            if (
                heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
            ) {
                const double gB = heap.size / MEM_CONSTANT;
                if ( highestGB < gB ) {
                    highestGB = gB;
                }
            }
        }

        if (
            highestGB >= MAX_HEAP_SIZE && deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU
        ) {
            Score += 3;
        }

        return Score;
    }

    VkExtent2D SwapchainManager::getSwapExtent(
        VkSurfaceCapabilities2KHR const& surfaceCap
    ) noexcept {
        if (
            surfaceCap.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()
        ) {
            return surfaceCap.surfaceCapabilities.currentExtent;
        }

        int width, height;
        SDL_GetWindowSizeInPixels(
            m_window,
            &width,
            &height
        );

        return {
            std::clamp<uint32_t>(width, surfaceCap.surfaceCapabilities.minImageExtent.width, surfaceCap.surfaceCapabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, surfaceCap.surfaceCapabilities.minImageExtent.height, surfaceCap.surfaceCapabilities.maxImageExtent.height)
        };
    }

    [[nodiscard]] VkResult LogicalDeviceDriver::init_instance() noexcept {
        if ( !m_window.init() ) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        const VkResult volkResult = volkInitialize();
        if ( volkResult != VK_SUCCESS ) {
            return volkResult;
        }

        uint32_t extCount { 0u };
        const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extCount);

        if ( !extensions ) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        const std::vector<const char*> validationLayer = {
            "VK_LAYER_KHRONOS_Validation"
        };

        uint32_t loaderVersion = VK_API_VERSION_1_0;
        const VkResult enumResult = vkEnumerateInstanceVersion(&loaderVersion);

        uint32_t requestedVersion = std::min(loaderVersion, VK_API_VERSION_1_4);

        if ( enumResult != VK_SUCCESS ) {
            requestedVersion = VK_API_VERSION_1_0;
        }

        userAPIVersion = requestedVersion;

        const VkApplicationInfo appInfo
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Djinn - Game Engine",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Djinn",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = requestedVersion
        };

        const VkDebugUtilsMessengerCreateInfoEXT debugInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = VK_Debug::debugCallback
        };

        const VkInstanceCreateInfo instCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &debugInfo,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(validationLayer.size()),
            .ppEnabledLayerNames = validationLayer.data(),
            .enabledExtensionCount = extCount,
            .ppEnabledExtensionNames = extensions
        };

        const VkResult instResult = vkCreateInstance(&instCreateInfo, &m_allocCallback, &m_instance);

        [[likely]] if ( instResult == VK_SUCCESS ) {
            volkLoadInstance(m_instance);
            if (
                SDL_Vulkan_CreateSurface(
                    m_window.window,
                    m_instance,
                    &m_allocCallback,
                    &m_surface
                )
            ) {
                return VK_SUCCESS;
            } else {
                return VK_ERROR_INITIALIZATION_FAILED;
            }
        } else [[unlikely]] {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    [[nodiscard]] bool LogicalDeviceDriver::init_physicalDevice() noexcept {
        uint32_t physDeviceCount { 0u };
        vkEnumeratePhysicalDevices(m_instance, &physDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physDevices(physDeviceCount);
        vkEnumeratePhysicalDevices(m_instance, &physDeviceCount, physDevices.data());

        std::vector<uint8_t> physDeviceScores(physDeviceCount);
        for ( const VkPhysicalDevice& physDevice : physDevices ) {
            VkPhysicalDeviceFeatures         deviceFeatures{};
            VkPhysicalDeviceProperties       deviceProps{};
            VkPhysicalDeviceMemoryProperties deviceMemProps{};

            vkGetPhysicalDeviceFeatures(physDevice, &deviceFeatures);
            vkGetPhysicalDeviceProperties(physDevice, &deviceProps);
            vkGetPhysicalDeviceMemoryProperties(physDevice, &deviceMemProps);

            physDeviceScores.push_back(
                scoreDeviceCapabilities(
                    deviceFeatures,
                    deviceProps,
                    deviceMemProps
                )
            );
        }

        const size_t Index = std::distance(physDeviceScores.begin(), std::max_element(physDeviceScores.begin(), physDeviceScores.end()));

        try {
            VkPhysicalDevice physDevice = physDevices.at(Index);
            VkPhysicalDeviceProperties deviceProperties{};

            vkGetPhysicalDeviceProperties(physDevice, &deviceProperties);
            
            userAPIVersion = deviceProperties.apiVersion;
            m_physicalDevice = physDevice;
            return true;
        } catch ( const std::out_of_range& except ) {
            VK_Debug::bad_event("Error in trying to get a physical device to use.");
            return false;
        }
    }

    [[nodiscard]] bool LogicalDeviceDriver::init_gfxQueue() noexcept {
        uint32_t gfxFamCount { 0u };
        vkGetPhysicalDeviceQueueFamilyProperties2(m_physicalDevice, &gfxFamCount, nullptr);
        std::vector<VkQueueFamilyProperties2> queueFamProps(gfxFamCount, 
        { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
        vkGetPhysicalDeviceQueueFamilyProperties2(m_physicalDevice, &gfxFamCount, queueFamProps.data());

        for (
            uint32_t Idx { 0u };
            Idx < gfxFamCount;
            Idx++
        ) {
            VkBool32 hasPresentSupported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                m_physicalDevice, Idx,
                 m_surface, &hasPresentSupported
            );

            const VkQueueFamilyProperties2& prop = queueFamProps[Idx];
            if (
                prop.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupported
            ) {
                m_gfxFamIdx = Idx;
                return true;
            }
        }
        VK_Debug::bad_event("Could not find surface support on physical device.");
        return false;
    }

    [[nodiscard]] bool LogicalDeviceDriver::init_logicalDevice() noexcept {
        float queuePriority { 1.f };
        std::vector<uint32_t> queueFamilies { m_gfxFamIdx };

        VkDeviceQueueCreateInfo deviceQueueCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_gfxFamIdx,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        VkPhysicalDeviceVulkan14Features features1_4
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            .pNext = nullptr
        };
        VkPhysicalDeviceVulkan13Features features1_3
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &features1_4
        };
        VkPhysicalDeviceVulkan12Features features1_2
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &features1_3
        };
        VkPhysicalDeviceFeatures2 physDeviceFeatures
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features1_2
        };
        vkGetPhysicalDeviceFeatures2(m_physicalDevice, &physDeviceFeatures);

        if (
            !features1_3.dynamicRendering || !features1_3.synchronization2 ||
            !features1_2.timelineSemaphore
        ) {
            hasDesiredFeatures = true;
        }

        featuresData[0] = features1_3.dynamicRendering;
        featuresData[1] = features1_3.synchronization2;
        featuresData[2] = features1_2.timelineSemaphore;

        const std::vector<const char*> deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo devCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &physDeviceFeatures,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueCreateInfo,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = nullptr
        };

        if (
            vkCreateDevice(
                m_physicalDevice,
                &devCreateInfo,
                &m_allocCallback,
                &m_logicalDevice
            ) != VK_SUCCESS
        ) {
            return false;
        }

        vkGetDeviceQueue(m_logicalDevice, m_gfxFamIdx, 0, &m_gfxQueue);

        if (!m_gfxQueue) {
            return false;
        }

        return true;
    }

    [[nodiscard]] bool SwapchainManager::init_Swapchain(
        VkDevice& logicalDevice,
        VkPhysicalDevice& physDevice,
        VkSurfaceKHR& surface,
        const VkAllocationCallbacks* allocator,
        VmaAllocator& vmaAlloc
    ) noexcept {
        VkSurfaceCapabilities2KHR surfaceCap
        {};

        const VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
            .pNext = nullptr,
            .surface = surface
        };

        const VkResult surfaceResult = vkGetPhysicalDeviceSurfaceCapabilities2KHR(
            physDevice,
            &surfaceInfo,
            &surfaceCap
        );

        if (
            surfaceResult != VK_SUCCESS
        ) {
            return false;
        }

        uint32_t surfaceFormatCount { 0u };

        vkGetPhysicalDeviceSurfaceFormats2KHR(
            physDevice,
            &surfaceInfo,
            &surfaceFormatCount,
            nullptr
        );
        std::vector<VkSurfaceFormat2KHR> surfaceFormats(surfaceFormatCount, {
            .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR
        });
        vkGetPhysicalDeviceSurfaceFormats2KHR(
            physDevice,
            &surfaceInfo,
            &surfaceFormatCount,
            surfaceFormats.data()
        );

        uint32_t surfacePresentModeCount { 0u };

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physDevice,
            surface,
            &surfacePresentModeCount,
            nullptr
        );
        std::vector<VkPresentModeKHR> presentModes(surfacePresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physDevice,
            surface,
            &surfacePresentModeCount,
            presentModes.data()
        );

        const std::vector<VkSurfaceFormat2KHR, std::allocator<VkSurfaceFormat2KHR>>::const_iterator Format_It = std::ranges::find_if(
            surfaceFormats, [](
                const VkSurfaceFormat2KHR& format
            ) -> bool {
                return format.surfaceFormat.format == desiredFormat && format.surfaceFormat.colorSpace == desiredColorSpace;
            }
        );

        if (
            Format_It == surfaceFormats.end()
        ) {
            try {
                m_surfaceFormat = surfaceFormats.at(0);
            } catch ( const std::out_of_range& err ) {
                VK_Debug::bad_event("An exception was catched when trying to access surfaceFormat's vector. There was none at position = 0.");
                return false;
            }
        } else {
            m_surfaceFormat = *Format_It;
        }

        if (
            !std::ranges::any_of(
                presentModes,
                [](const VkPresentModeKHR& currMode) {
                    return currMode == VK_PRESENT_MODE_FIFO_KHR;
                }
            )
        ) {
            return false;
        }

        const VkPresentModeKHR presentMode = std::ranges::any_of(presentModes,
        [](const VkPresentModeKHR& currMode){
            return desiredPresentMode == currMode;
        }) ? desiredPresentMode : VK_PRESENT_MODE_FIFO_KHR;

        VkExtent2D swapchainExtent = getSwapExtent(
            surfaceCap
        );

        uint32_t minImageCount = std::max(3u, surfaceCap.surfaceCapabilities.minImageCount);
        if ( (0 < surfaceCap.surfaceCapabilities.maxImageCount) && (surfaceCap.surfaceCapabilities.maxImageCount < minImageCount)) {
            minImageCount = surfaceCap.surfaceCapabilities.maxImageCount;
        }

        const VkSwapchainCreateInfoKHR swapChainCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = minImageCount,
            .imageFormat = m_surfaceFormat.surfaceFormat.format,
            .imageColorSpace = m_surfaceFormat.surfaceFormat.colorSpace,
            .imageExtent = swapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = surfaceCap.surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = true
        };

        if (
            vkCreateSwapchainKHR(
                logicalDevice,
                &swapChainCreateInfo,
                allocator,
                &m_swapChain
            ) != VK_SUCCESS
        ) {
            return false;
        }

        uint32_t imageCount { 0u };
        vkGetSwapchainImagesKHR(
            logicalDevice,
            m_swapChain,
            &imageCount,
            nullptr
        );

        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(
            logicalDevice,
            m_swapChain,
            &imageCount,
            m_images.data()
        );
        m_imageViews.resize(imageCount);

        size_t idx { 0uz };
        for (
            VkImage& currImage : m_images
        ) {
            const VkImageViewCreateInfo imgViewInfo
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = currImage,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_surfaceFormat.surfaceFormat.format,
                .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1
                }
            };

            if (
                vkCreateImageView(
                    logicalDevice,
                    &imgViewInfo,
                    allocator,
                    &m_imageViews[idx]
                ) != VK_SUCCESS
            ) {
                return false;
            }
            idx++;
        }

        renderCompleteSemaphores.resize(m_images.size());
        for (
            VkSemaphore& semaphore : renderCompleteSemaphores
        ) {
            const VkSemaphoreCreateInfo semCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
            };
            if (
                vkCreateSemaphore(
                    logicalDevice,
                    &semCreateInfo,
                    allocator,
                    &semaphore
                ) != VK_SUCCESS
            ) {
                return false;
            }
        }

        const VkImageCreateInfo depthImageCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = depthFormat,
            .extent
            {
                .width = swapchainExtent.width,
                .height = swapchainExtent.height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        const VmaAllocationCreateInfo allocInfo
        {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        if (
            vmaCreateImage(
                vmaAlloc,
                &depthImageCreateInfo,
                &allocInfo,
                &m_depthImage,
                &m_depthImageAllocation,
                nullptr
            ) != VK_SUCCESS
        ) {
            return false;
        }

        const VkImageViewCreateInfo depthImgViewCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_depthImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = depthFormat,
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        };

        if (
            vkCreateImageView(
                logicalDevice,
                &depthImgViewCreateInfo,
                allocator,
                &m_depthImageView
            ) != VK_SUCCESS
        ) {
            return false;
        } else {
            return true;
        }
    }

    void SwapchainManager::shutdown(
        VkDevice& logicalDevice,
        const VkAllocationCallbacks* allocator,
        VmaAllocator& vmaAlloc
    ) noexcept {
        for (
            VkImageView& imgView : m_imageViews
        ) {
            vkDestroyImageView(
                logicalDevice,
                imgView,
                allocator
            );
        }
        m_imageViews.clear();

        for (
            VkSemaphore& semaphore : renderCompleteSemaphores
        ) {
            vkDestroySemaphore(
                logicalDevice,
                semaphore,
                allocator
            );
        }
        renderCompleteSemaphores.clear();

        if (
            m_swapChain
        ) {
            vkDestroySwapchainKHR(
                logicalDevice,
                m_swapChain,
                allocator
            );
            m_swapChain = nullptr;
        }

        if (
            m_depthImageView
        ) {
            vkDestroyImageView(
                logicalDevice,
                m_depthImageView,
                allocator
            );
            vmaDestroyImage(
                vmaAlloc,
                m_depthImage,
                m_depthImageAllocation
            );
            m_depthImageView = nullptr;
        }
    }

    [[nodiscard]] bool PipelineManager::init_Pipeline(
        std::inplace_vector<std::tuple<VkShaderModule*, ShaderType>, 5>& shaderVector
    ) noexcept {
        std::inplace_vector<VkPipelineShaderStageCreateInfo, 5> shadersCreateInfo;
        for ( std::tuple<VkShaderModule*, ShaderType>& element : shaderVector ) {
            ShaderType shaderType = std::get<1>(element);
            if (
                shaderType == ShaderType::Vertex
            ) {
                VkPipelineShaderStageCreateInfo shaderPipelineCreateInfo
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = *std::get<0>(element),
                    .pName = "vertexMain"
                };

                shadersCreateInfo.push_back(
                    shaderPipelineCreateInfo
                );
            } else if (
                shaderType == ShaderType::Fragment
            ) {
                VkPipelineShaderStageCreateInfo shaderPipelineCreateInfo
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = *std::get<0>(element),
                    .pName = "fragMain"
                };

                shadersCreateInfo.push_back(
                    shaderPipelineCreateInfo
                );
            }
        }

        const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo
        {
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        };
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo
        {
            .viewportCount = 1,
            .scissorCount = 1
        };

        VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo
        {
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = 
        };
    }
}