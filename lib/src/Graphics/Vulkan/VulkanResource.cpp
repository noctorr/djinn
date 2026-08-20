#include "VulkanResource.hpp"
#include "VulkanDebug.hpp"
#include "Window.hpp"
#include <vector>

VkResult Djinn::Instance::initInstance() noexcept
{
    if ( volkInitialize() != VK_SUCCESS )
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkApplicationInfo appInfo
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Djinn - Game Engine Editor",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "Djinn",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = vulkanAPIVersion
    };

    uint32_t instExtCount { 0u };
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&instExtCount);

    if ( !extensions )
    {
        return VK_INCOMPLETE;
    }

    std::vector<const char*> requestedLayers { "VK_LAYER_KHRONOS_Validation" };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VKDebug::debugCallback
    };

    VkInstanceCreateInfo instanceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debugCreateInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = instExtCount,
        .ppEnabledExtensionNames = extensions
    };

    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    if ( result != VK_SUCCESS )
    {
        return result;
    } else
    {
        volkLoadInstance(instance);

        if ( SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface) )
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        } else
        {
            instPointer = &instance;
            return result;
        }
    }
}

bool Djinn::PhysicalDevice::initPhysicalDevice() noexcept
{
    uint32_t physDeviceCount { 0u };
    vkEnumeratePhysicalDevices(*instPointer, &physDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physDeviceCount);
    vkEnumeratePhysicalDevices(*instPointer, &physDeviceCount, physicalDevices.data());

    VkPhysicalDevice physicalDevice { VK_NULL_HANDLE };
    VkPhysicalDeviceType physDeviceType;

    if (physDeviceCount)
    {
        physicalDevice = physicalDevices[0];
        VkPhysicalDeviceProperties physDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physDeviceProperties);
        physDeviceType = physDeviceProperties.deviceType;
        for ( const VkPhysicalDevice& physDevice : physicalDevices )
        {
            VkPhysicalDeviceProperties deviceProps{};
            vkGetPhysicalDeviceProperties(physDevice, &deviceProps);
            if (
                deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            ) {
                physicalDevice = physDevice;
                physDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            } else if (
                deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
                physDeviceType == VK_PHYSICAL_DEVICE_TYPE_CPU
            ) {
                physicalDevice = physDevice;
                physDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
            }
        }

        VKDebug::event("Physical device has been set.");
        VKDebug::physdevice_type_event(physDeviceType);

        return true;
    } else
    {
        VKDebug::event("There is zero physical devices that was detected.");
        return false;
    }
}
