#include "Logical_Device_Driver.hpp"
#include <SDL3/SDL_vulkan.h>

#include "Vulkan_Debug.hpp"

#include <vector>
#include <algorithm>
#include <iterator>

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

        UserAPIVersion = requestedVersion;

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

        const VkResult instResult = vkCreateInstance(&instCreateInfo, nullptr, &m_instance);

        [[likely]] if ( instResult == VK_SUCCESS ) {
            volkLoadInstance(m_instance);
            maxAPICheck();

            if (
                SDL_Vulkan_CreateSurface(
                    m_window.window,
                    m_instance,
                    nullptr,
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

        const auto Index = std::distance(physDeviceScores.begin(), std::max_element(physDeviceScores.begin(), physDeviceScores.end()));

        try {
            VkPhysicalDevice physDevice = physDevices.at(Index);
            VkPhysicalDeviceProperties deviceProperties{};

            vkGetPhysicalDeviceProperties(physDevice, &deviceProperties);
            
            UserAPIVersion = deviceProperties.apiVersion;
            
            return true;
        } catch ( const std::out_of_range& except ) {

        }
    }
}