#include "Application.hpp"

bool Engine::Application::initialise() noexcept
{
    if (
        !Djinn::createWindow()
    ) {
        return false;
    }

    if (
        m_instance.initInstance() != VK_SUCCESS
    ) {
        return false;
    }

    if (
        !m_physDevice.initPhysicalDevice()
    ) {
        return false;
    }

    if (
        !m_pipeline.initGFXQueue()
    ) {
        return false;
    }

    if (
        !m_logicalDevice.initDevice()
    ) {
        return false;
    }

    if (
        !m_logicalDevice.initVMA()
    ) {
        return false;
    }

    if (
        !m_swapchain.initSwapchain(
            Djinn::width,
            Djinn::height
        )
    ) {
        return false;
    }

    if (
        !m_pipeline.initShaders()
    ) {
        return false;
    }

    if (
        !m_pipeline.initGFXPipeline()
    ) {
        return false;
    }

    if (
        !m_logicalDevice.initSynchronisation()
    ) {
        return false;
    }

    if (
        !m_logicalDevice.initCommandBuffers()
    ) {
        return false;
    }

    return true;
}

void Engine::Application::shutdown() noexcept
{
    
}