#include "Application.hpp"
#include "VulkanDebug.hpp"
#include "VulkanAlloc.hpp"

#include <FileReader.hpp>

bool Djinn::Application::initialiseApplication() noexcept
{
    if ( !SDL_InitSubSystem(SDL_INIT_VIDEO) )
    {
        SDL_reportError(SDL_GetError());
        return false;
    }

    m_window = SDL_CreateWindow(
        "Djinn",
        Djinn::windowWidth, Djinn::windowHeight,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if ( !m_window )
    {
        SDL_reportError(SDL_GetError());
        return false;
    }

    return initialiseVulkan();
}

VkResult Djinn::Application::p_initInstance() noexcept
{
    if ( volkInitialize() != VK_SUCCESS )
    {
        SDL_reportError("Error occured when initialising Volk.");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkApplicationInfo appInfo
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Djinn - Game Engine Editor",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Djinn",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = Djinn::vulkanVersion
    };

    uint32_t instExtCount { 0u };
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&instExtCount);

    if ( !extensions )
    {
        return VK_INCOMPLETE;
    }

    std::vector<const char*> requestedLayers { "VK_LAYER_KHRONOS_Validation" };

    VkDebugUtilsMessengerCreateInfoEXT debugInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VKDebug::debugCallback
    };

    VkInstanceCreateInfo instCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debugInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = instExtCount,
        .ppEnabledExtensionNames = extensions
    };

    VkResult result = vkCreateInstance(&instCreateInfo, nullptr, &m_instance);

    if ( result != VK_SUCCESS )
    {
        return result;
    } else
    {
        volkLoadInstance(m_instance);

        if ( !SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface) )
        {
            return VK_ERROR_INITIALIZATION_FAILED;
        } else
        {
            return result;
        }
    }
}

bool Djinn::Application::p_initPhysicalDevices() noexcept
{
    uint32_t physDeviceCount{};
    vkEnumeratePhysicalDevices(m_instance, &physDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physDevices(physDeviceCount);
    vkEnumeratePhysicalDevices(m_instance, &physDeviceCount, physDevices.data());

    VkPhysicalDevice physicalDevice { VK_NULL_HANDLE };
    VkPhysicalDeviceType physDeviceType;

    if (physDeviceCount)
    {
        physicalDevice = physDevices[0];
        VkPhysicalDeviceProperties deviceProp{};
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProp);
        physDeviceType = deviceProp.deviceType;
        for ( const auto& physDevice : physDevices )
        {
            VkPhysicalDeviceProperties deviceProps{};
            vkGetPhysicalDeviceProperties(physDevice, &deviceProps);
            if ( deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
            {
                physicalDevice = physDevice;
                physDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            } else if ( deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && 
                physDeviceType == VK_PHYSICAL_DEVICE_TYPE_CPU )
            {
                physicalDevice = physDevice;
                physDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
            }
        }
    }


}

bool Djinn::Application::initialiseVulkan() noexcept
{

}