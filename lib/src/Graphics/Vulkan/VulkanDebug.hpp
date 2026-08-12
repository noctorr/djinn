#include "Application.hpp"
#include <iostream>

namespace VKDebug {
    inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        if ( messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
        {
            std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }
}