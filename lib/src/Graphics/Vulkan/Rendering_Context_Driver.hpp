#pragma once

#include "Logical_Device_Driver.hpp"

namespace Engine {
    class RenderContextDriver final {
        Djinn_Vulkan::LogicalDeviceDriver m_driver{};

        VmaAllocator m_allocator                { VK_NULL_HANDLE };
        public:

        private:
        [[nodiscard]] bool initVMA() noexcept;

    };
}