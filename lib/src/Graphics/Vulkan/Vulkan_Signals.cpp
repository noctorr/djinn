#include "Vulkan_Signals.hpp"

namespace Djinn_Vulkan {
    VulkanSignal* VulkanSignal::m_singleton { nullptr };
    VulkanSignal::VulkanSignal()
    {
        if (!m_singleton) {
            m_singleton = this;
        }
    }

    VulkanSignal::~VulkanSignal()
    {
        if ( m_singleton == this ) {
            m_singleton = nullptr;
        }
    }
}