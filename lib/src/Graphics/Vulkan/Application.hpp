#pragma once
#include "VulkanResource.hpp"


namespace Engine
{
    class Application final
    {
        Djinn::Instance m_instance;
        Djinn::PhysicalDevice m_physDevice;
        Djinn::Device m_logicalDevice;
        Djinn::Swapchain m_swapchain;
        Djinn::Pipeline m_pipeline;

        bool m_running { false };
        public:
        void run() noexcept;
        void render() noexcept;
        void shutdown() noexcept;
        bool initialise() noexcept;
    };
}