#include "Rendering_Context_Driver.hpp"

namespace Engine {
    [[nodiscard]] bool RenderContextDriver::initVMA() noexcept {
        VmaVulkanFunctions vmaFuncInfo{};
        VmaAllocatorCreateInfo vmaCreateInfo
        {
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = m_driver.getPhysicalDevice(),
            .device = m_driver.getLogicalDevice(),
            .pVulkanFunctions = &vmaFuncInfo,
            .instance = m_driver.getInstance(),
            .vulkanApiVersion = m_driver.m_userAPIVersion
        };

        vmaImportVulkanFunctionsFromVolk(&vmaCreateInfo, &vmaFuncInfo);

        if (
            vmaCreateAllocator(
                &vmaCreateInfo,
                &m_allocator
            ) != VK_SUCCESS
        ) {
            return false;
        } else {
            return true;
        }
    }
}