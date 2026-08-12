#include "Application.hpp"
#include "VulkanDebug.hpp"

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

VkResult Djinn::Application::p_createInstance() noexcept
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
        return result;
    }
}

bool Djinn::Application::initialiseVulkan() noexcept
{

}