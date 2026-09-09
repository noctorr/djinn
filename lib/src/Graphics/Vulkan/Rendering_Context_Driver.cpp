#include "Rendering_Context_Driver.hpp"
#include "FileReader.hpp"

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
            .vulkanApiVersion = m_driver.userAPIVersion
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

    [[nodiscard]] bool RenderContextDriver::createShader(
        const std::filesystem::path& filePath,
        const shaderc_shader_kind shaderKind,
        VkShaderModule* shaderModule
    ) noexcept {
        const std::optional<std::string> shaderFile = ftl::readFile(filePath);

        if (
            shaderFile == std::nullopt
        ) {
            return false;
        }

        shaderc::Compiler compiler;
        shaderc::CompileOptions compilerOpts;
        compilerOpts.SetGenerateDebugInfo();
        compilerOpts.SetOptimizationLevel(shaderc_optimization_level_performance);
        compilerOpts.SetTargetEnvironment(shaderc_target_env_vulkan, m_driver.userAPIVersion);

        shaderc::CompilationResult compilerResult = compiler.CompileGlslToSpv(shaderFile.value(), shaderKind, filePath.c_str(), compilerOpts);

        if (
            compilerResult.GetCompilationStatus() != shaderc_compilation_status_success
        ) {
            return false;
        }

        std::vector<uint32_t> spv = { compilerResult.cbegin(), compilerResult.cend() };

        const VkShaderModuleCreateInfo shaderCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = spv.size() * sizeof(uint32_t),
            .pCode = spv.data()
        };

        if (
            vkCreateShaderModule(
                m_driver.getLogicalDevice(),
                &shaderCreateInfo,
                m_driver.getAllocCallback(),
                shaderModule
            ) != VK_SUCCESS
        ) {
            return false;
        } else {
            return true;
        }
    }

    [[nodiscard]] bool RenderContextDriver::initShaders() noexcept {
        if (
            createShader(
                "src/Graphics/Modules/glsl/default.vert",
                shaderc_vertex_shader,
                &m_vertexShader
            ) &&
            createShader(
                "src/Graphics/Modules/glsl/default.frag",
                shaderc_fragment_shader,
                &m_fragmentShader
            )
        ) {
            return true;
        } else {
            return false;
        }
    }
}