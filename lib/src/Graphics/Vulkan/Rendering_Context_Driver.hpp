#pragma once

#include "Logical_Device_Driver.hpp"
#include <filesystem>

namespace Engine {
    class RenderContextDriver final {
        Djinn_Vulkan::LogicalDeviceDriver m_driver{};


        VmaAllocator m_allocator                { VK_NULL_HANDLE };

        VkShaderModule m_vertexShader           { VK_NULL_HANDLE };
        VkShaderModule m_fragmentShader         { VK_NULL_HANDLE };
        public:

        private:
        [[nodiscard]] bool initVMA() noexcept;
        [[nodiscard]] bool initShaders() noexcept;
        [[nodiscard]] bool createShader(
            const std::filesystem::path&,
            const shaderc_shader_kind,
            VkShaderModule*
        ) noexcept;
    };
}